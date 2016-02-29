#include "BLM.h"

#include "Midi.h"

#include <cstring>

BLM::BLM() :
    _colors(2),
    _cols(16),
    _rows(16),
    _controller(nullptr)
{
    std::memset(_buttonState, sizeof(_buttonState), 0);
    Midi::addDevice(this, "Launchpad");
}

BLM::~BLM()
{
    Midi::removeDevice(this);
}

void BLM::connected()
{
    std::cout << "BLM connected!" << std::endl;
}

void BLM::disconnected()
{
    std::cout << "BLM disconnected!" << std::endl;
}

void BLM::handleMessage(const MidiMessage &msg)
{
    //std::cout << "BLM input: " << msg << std::endl;

    handleBlmMessage(msg);
}

int BLM::buttonState(int col, int row) const
{
    return _buttonState[col][row];
}

void BLM::setButtonState(int col, int row, int state)
{
    _buttonState[col][row] = state;
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

        // TODO midiDataReceived = true;
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

    // TODO midiDataReceived = true;
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
                // no error checking... just send layout (the hardware version will check better)
                //sendBLMLayout();
            } else if (data[6] == 0x0f && data[7] == 0xf7) {
                //sendAck();
            }
        }
    } break;
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
    MidiMessage msg(sysex);
    sendMessage(msg);
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
    MidiMessage msg(sysex);
    sendMessage(msg);
}

void BLM::dump()
{
    for (int row = 0; row < _rows; ++row) {
        for (int col = 0; col < _cols; ++col) {
            std::cout << _buttonState[col][row] << "  ";
        }
        std::cout << std::endl;
    }
}
