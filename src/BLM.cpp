#include "BLM.h"
#include "Midi.h"
#include "Debug.h"
#include "Settings.h"

#include <oscpkt/oscpkt.h>

#include <cstring>

static const int idlePattern[] = {
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,   0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,   0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,   0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,   0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,   0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,   0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,   0, 0, 1, 1, 1, 1, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0,   0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,   0, 0, 1, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,   0, 0, 1, 0, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0,   0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,   0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,   0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0,   0, 0, 0, 0, 1, 0, 0, 0,
};

BLM::BLM() :
    _controller(nullptr),
    _layout(Full),
    _state(WaitController),
    _blmReady(false),
    _controllerReady(false),
    _colors(2),
    _cols(16),
    _rows(8),
    _idleCounter(0)
{
    std::memset(_buttonState, 0, sizeof(_buttonState));

    // Read protocol configuration and setup MIDI or OSC protocol
    Settings &settings = Settings::instance();
    std::string protocol = settings.json()["blm"]["protocol"].string_value();
    if (protocol == "midi") {
        _protocol = MIDI;
        std::string port = settings.json()["blm"]["midi"]["port"].string_value();
        if (port.empty()) {
            settings.error("blm.midi.port", "Empty or missing MIDI port configuration!");
        }
        Midi::addDevice(this, port);
    } else if (protocol == "osc") {
        _protocol = OSC;
        std::string remoteHost = settings.json()["blm"]["osc"]["remoteHost"].string_value();
        if (remoteHost.empty()) {
            settings.error("blm.osc.remoteHost", "Empty or missing OSC remote host configuration!");
        }
        int remotePort = settings.json()["blm"]["osc"]["remotePort"].int_value();
        if (remotePort == 0) {
            settings.error("blm.osc.remotePort", "Empty or missing OSC remote port configuration!");
        }
        int localPort = settings.json()["blm"]["osc"]["localPort"].int_value();
        if (localPort == 0) {
            settings.error("blm.osc.localPort", "Empty or missing OSC local port configuration!");
        }
        if (!_socket.connect(remoteHost, localPort, remotePort)) {
            throw Exception("Failed to open socket (remoteHost=%s, localPort=%d, remotePort=%d)", remoteHost, localPort, remotePort);
        }
        _oscTimer = startTimer(1);
    } else {
        settings.error("blm.protocol", "Invalid protocol configuration, expecting either \"midi\" or \"osc\"!");
    }

    _updateTimer = startTimer(1000);
    update();
    //_idleTimer = startTimer(25);
    //_ackTimer = startTimer(5000);
}

BLM::~BLM()
{
    if (_protocol == MIDI) {
        Midi::removeDevice(this);
    }
}

void BLM::setController(Controller *controller)
{
    if (_controller) {
        _controller->clearLeds();
        _controller->setBLM(nullptr);
    }
    _controller = controller;
    if (_controller) {
        _controller->setBLM(this);
    }
}

void BLM::controllerConnected(Layout layout)
{
    DBG("Controller connected");
    _controllerReady = true;
    setLayout(layout);
}

void BLM::controllerDisconnected()
{
    DBG("Controller disconnected");
    _controllerReady = false;
}

void BLM::setLayout(Layout layout)
{
    _layout = layout;
    _cols = 16;
    _rows = _layout == Half ? 8 : 16;
}

int BLM::buttonState(int col, int row) const
{
    return _buttonState[col][row];
}

void BLM::setButtonState(int col, int row, int state)
{
    _buttonState[col][row] = state;
    if (_state == Running) {
        sendButtonState(col, row, state);
    }
}

void BLM::sendNoteEvent(int channel, int note, int velocity)
{
    if (_state == Running) {
        sendMessageActiveProtocol(MidiMessage::noteOn(channel, note, velocity));
    }
}

void BLM::dump()
{
    for (int row = 0; row < MaxRows; ++row) {
        for (int col = 0; col < MaxCols; ++col) {
            std::cout << _buttonState[col][row] << "  ";
        }
        std::cout << std::endl;
    }
}

void BLM::connected()
{
    DBG("BLM connected via MIDI");
}

void BLM::disconnected()
{
    DBG("BLM disconnected via MIDI");
    _blmReady = false;
}

void BLM::handleMessage(const MidiMessage &msg)
{
    DBG("BLM received %s", msg);

    handleBlmMessage(msg);
}

void BLM::handleTimer(int id)
{
    if (id == _updateTimer) {
        update();
    } else  if (id == _idleTimer) {
        updateIdlePattern();
    } else if (id == _oscTimer) {
        handleOscInput();
    }
}

void BLM::setLed(int col, int row, int bit, int enabled)
{
    int mask = 1 << bit;
    int state = buttonState(col, row) & ~mask;
    if (enabled) {
        state |= mask;
    }
    setButtonState(col, row, state);
}

void BLM::setLedPattern8_H(int colOffset, int row, int bit, int pattern)
{
    int mask = 1 << bit;

    for (int i = 0; i < 8; ++i) {
        int col = colOffset + i;
        int state = buttonState(col, row) & ~mask;
        if (pattern & (1 << i)) {
            state |= mask;
        }
        setButtonState(col, row, state);
    }
}

void BLM::setLedPattern8_V(int col, int rowOffset, int bit, int pattern)
{
    int mask = 1 << bit;

    for (int i = 0; i < 8; ++i) {
        int row = rowOffset + i;
        int state = buttonState(col, row) & ~mask;
        if (pattern & (1 << i)) {
            state |= mask;
        }
        setButtonState(col, row, state);
    }
}

void BLM::handleBlmMessage(const MidiMessage &msg)
{
    int event = msg.event();
    int channel = msg.channel();
    const auto &data = msg.data();
    int data1 = data[1];
    int data2 = data[2];

    switch (event) {
    case 0x8: // NoteOff
        data2 = 0; // handle like NoteOn with velocity 0
    case 0x9: {
        int state;
        if (data2 == 0) {
            state = 0;
        } else if (data2 < 0x40) {
            state = 1;
        } else if (data2 < 0x60) {
            state = 2;
        } else {
            state = 3;
        }

        if (data1 < _cols) {
            int row = channel;
            int col = data1;
            setButtonState(col, row, state);
        } else if (data1 == 0x40) {
            setButtonState(MaxColsExtraOffset, channel, state);
        } else if (channel == 0 && data1 >= 0x60 && data1 <= 0x6f) {
            setButtonState(data1-0x60, MaxRowsExtraOffset, state);
        } else if (channel == 15 && data1 == 0x60) {
            setButtonState(MaxColsExtraOffset, MaxRowsExtraOffset, state);
        }

        _blmReady = true;
    } break;
    case 0xb: {
        int pattern = data2;
        if (data1 & 0x01) {
            pattern |= (1 << 7);
        }

        switch (data1 & 0xfe) {
        // 16x16 green
        case 0x10: setLedPattern8_H(0, channel, 0, pattern); break;
        case 0x12: setLedPattern8_H(8, channel, 0, pattern); break;

        // 16x16 green rotated
        case 0x18: setLedPattern8_V(channel, 0, 0, pattern); break;
        case 0x1a: setLedPattern8_V(channel, 8, 0, pattern); break;

        // 16x16 red
        case 0x20: setLedPattern8_H(0, channel, 1, pattern); break;
        case 0x22: setLedPattern8_H(8, channel, 1, pattern); break;

        // 16x16 red rotated
        case 0x28: setLedPattern8_V(channel, 0, 1, pattern); break;
        case 0x2a: setLedPattern8_V(channel, 8, 1, pattern); break;

        // extra column green
        case 0x40: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset, 0, 0, pattern); break;
        case 0x42: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset, 8, 0, pattern); break;

        // extra column red
        case 0x48: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset, 0, 1, pattern); break;
        case 0x4a: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset, 8, 1, pattern); break;

        // extra shift column green
        case 0x50: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset+1, 0, 0, pattern); break;
        case 0x52: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset+1, 8, 0, pattern); break;

        // extra shift column red
        case 0x58: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset+1, 0, 1, pattern); break;
        case 0x5a: if (channel == 0) setLedPattern8_V(MaxColsExtraOffset+1, 8, 1, pattern); break;

        // extra row green & shift
        case 0x60:
            if (channel == 0) setLedPattern8_H(0, MaxRowsExtraOffset, 0, pattern);
            else if (channel == 15) setLed(MaxColsExtraOffset, MaxRowsExtraOffset, 0, pattern & 1);
            break;
        case 0x62: if (channel == 0) setLedPattern8_H(8, MaxRowsExtraOffset, 0, pattern); break;

        // extra row red & shift
        case 0x68:
            if (channel == 0) setLedPattern8_H(0, MaxRowsExtraOffset, 1, pattern);
            else if (channel == 15) setLed(MaxColsExtraOffset, MaxRowsExtraOffset, 1, pattern & 1);
            break;

        case 0x6a: if (channel == 0) setLedPattern8_H(8, MaxRowsExtraOffset, 1, pattern); break;
        }

        _blmReady = true;
    } break;

    case 0xf: {
        // in the hope that SysEx messages will always be sent in a single packet...
        if (data.size() >= 8 &&
            data[0] == 0xf0 &&
            data[1] == 0x00 &&
            data[2] == 0x00 &&
            data[3] == 0x7e &&
            data[4] == 0x4e && // MBHP_BLM_SCALAR
            data[5] == 0x00)   // Device ID
        {
            if (data[6] == 0x00 && data[7] == 0x00) {
                _blmReady = true;
                // no error checking... just send layout (the hardware version will check better)
                sendLayout();
            } else if (data[6] == 0x0f && data[7] == 0xf7) {
                _blmReady = true;
                sendAck();
            }
        }
    } break;
    }
}

void BLM::handleOscInput()
{
    uint8_t data[1024];
    int len = _socket.read(data, sizeof(data));
    if (len > 0) {
        oscpkt::PacketReader reader(data, len);
        if (reader.isOk()) {
            while (oscpkt::Message *msg = reader.popMessage()) {
                uint32_t midi;
                if (msg->partialMatch("/midi").popMidi(midi).isOkNoMoreArgs()) {
                    MidiMessage midiMsg((midi >> 24) & 0xff, (midi >> 16) & 0xff, (midi >> 8) & 0xff);
                    handleBlmMessage(midiMsg);
                }
            }
        }
    }
}

void BLM::sendButtonState(int col, int row, int state)
{
    if (col < MaxColsExtraOffset && row < MaxRowsExtraOffset) {
        _controller->setGridLed(col, row, state);
    } else if (col == MaxColsExtraOffset && row < MaxRowsExtraOffset) {
        _controller->setExtraColumnLed(0, row, state);
    } else if (col == (MaxColsExtraOffset+1) && row < MaxRowsExtraOffset) {
        _controller->setExtraColumnLed(1, row, state);
    } else if (col < MaxColsExtraOffset && row == MaxRowsExtraOffset) {
        _controller->setExtraRowLed(col, 0, state);
    }
}

void BLM::sendLayout()
{
    std::vector<uint8_t> sysex(14);
    sysex[0] = 0xf0;
    sysex[1] = 0x00;
    sysex[2] = 0x00;
    sysex[3] = 0x7e;
    sysex[4] = 0x4e; // MBHP_BLM_SCALAR ID
    sysex[5] = 0x00; // Device ID 00
    sysex[6] = 0x01; // Command #1 (Layout Info)
    sysex[7] = _cols & 0x7f; // number of columns
    sysex[8] = _rows & 0x7f; // number of rows
    sysex[9] = _colors & 0x7f; // number of LED colours
    sysex[10] = 1; // number of extra rows
    sysex[11] = 1; // number of extra columns
    sysex[12] = 1; // number of extra buttons (e.g. shift)
    sysex[13] = 0xf7;
    sendMessageActiveProtocol(MidiMessage(sysex));
}

void BLM::sendAck()
{
    std::vector<uint8_t> sysex(9);
    sysex[0] = 0xf0;
    sysex[1] = 0x00;
    sysex[2] = 0x00;
    sysex[3] = 0x7e;
    sysex[4] = 0x4e; // MBHP_BLM_SCALAR ID
    sysex[5] = 0x00; // Device ID 00
    sysex[6] = 0x0f; // Acknowledge
    sysex[7] = 0x00; // dummy
    sysex[8] = 0xf7;
    sendMessageActiveProtocol(MidiMessage(sysex));
}

void BLM::sendMessageActiveProtocol(const MidiMessage &msg)
{
    if (_protocol == MIDI) {
        sendMessage(msg);
    } else if (_protocol == OSC) {
        int oscPort = 0;
        oscpkt::Message oscMsg(tfm::format("/midi%d", oscPort + 1));

        const auto &data = msg.data();

        if (msg.isSysex()) {
            oscMsg.pushBlob((void *)(data.data()), data.size());
        } else {
            uint32_t midi = 0;
            for (int i = 0; i < 3; ++i) {
                midi |= data[i] << ((3 - i) * 8);
            }
            oscMsg.pushMidi(midi);
        }

        oscpkt::PacketWriter writer;
        writer.addMessage(oscMsg);
        bool ok = _socket.write((const unsigned char *)writer.packetData(), writer.packetSize());
        if (!ok) {
            DBG("failed to send OSC packet");
        }
    }
}

void BLM::update()
{
    switch (_state) {
    // Wait for controller to connect
    case WaitController:
        if (_idleTimer == 0) {
            _idleTimer = startTimer(25);
        }
        if (_controllerReady) {
            _blmReady = false;
            sendLayout();
            sendAck();
            _state = WaitBlm;
        }
        break;
    // Wait for BLM to connect
    case WaitBlm:
        if (_controllerReady && _blmReady) {
            if (_idleTimer != 0) {
                stopTimer(_idleTimer);
                _idleTimer = 0;
            }
            _controller->clearLeds();
            for (int row = 0; row < MaxRows; ++row) {
                for (int col = 0; col < MaxCols; ++col) {
                    sendButtonState(col, row, _buttonState[col][row]);
                }
            }
            _state = Running;
        } else {
            _state = WaitController;
        }
        break;
    // Running
    case Running:
        if (_controllerReady && _blmReady) {
            sendAck();
        } else {
            _state = WaitController;
        }
        break;
    }
}

void BLM::updateIdlePattern()
{
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int state = idlePattern[y * 16 + x];
            if (_idleCounter % 16 == y) {
                state = state ? 1 : 2;
            }
            sendButtonState(x, y, state);
        }
        sendButtonState(MaxRowsExtraOffset, y, _idleCounter % 16 == y ? 2 : 0);
        sendButtonState(MaxRowsExtraOffset + 1, y, _idleCounter % 16 == y ? 2 : 0);
        sendButtonState(y, MaxColsExtraOffset, _idleCounter % 16 == 0 ? 2 : 0);
    }

    ++_idleCounter;
}
