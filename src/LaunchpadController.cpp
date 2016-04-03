#include "LaunchpadController.h"
#include "LaunchpadDevice.h"
#include "Debug.h"
#include "BLM.h"
#include "MidiMessage.h"
#include "Settings.h"

LaunchpadController::LaunchpadController()
{
    Settings &settings = Settings::instance();
    _count = settings.json()["launchpad"]["count"].int_value();
    if (_count != 2 && _count != 4) {
        settings.error("launchpad.count", "Missing or invalid number of devices, needs to be either 2 or 4!");
    }
    for (int i = 0; i < _count; ++i) {
        std::string section = tfm::format("launchpad%d", i);
        std::string port = settings.json()["launchpad"][section]["port"].string_value();
        if (port.empty()) {
            settings.error(tfm::format("launchpad.%s.port", section), "Empty or missing MIDI port configuration!");
        }
        int rotation = settings.json()["launchpad"][section]["rotation"].int_value();
        if (rotation != 0 && rotation != 90 && rotation != 180 && rotation != 270) {
            settings.error(tfm::format("launchpad.%s.rotation", section), "Invalid rotation configuration, expecting 0, 90, 180 or 270!");
        }
        _devices.emplace_back(new LaunchpadDevice(this, i, port, LaunchpadDevice::Rotation(rotation)));
    }
}

LaunchpadController::~LaunchpadController()
{
}

void LaunchpadController::deviceConnected(LaunchpadDevice *device)
{
    DBG("Launchpad %d connected", device->index());
    for (const auto &d : _devices) {
        if (!d->isConnected()) {
            return;
        }
    }
    DBG("Launchpads ready!");
    switch (_count) {
    case 2: _blm->controllerConnected(BLM::Half); break;
    case 4: _blm->controllerConnected(BLM::Full); break;
    }
}

void LaunchpadController::deviceDisconnected(LaunchpadDevice *device)
{
    DBG("Launchpad %d disconnected", device->index());
    _blm->controllerDisconnected();
}

void LaunchpadController::deviceMessage(LaunchpadDevice *device, const MidiMessage &msg)
{
    DBG("Launchpad %d received %s", device->index(), msg);

    int deviceIndex = device->index();

    int event = msg.event();
    int channel = msg.channel();
    const auto &data = msg.data();
    int data1 = data[1];
    int data2 = data[2];

    if (event == 0x09 && channel == 0) {
        if ((data1 & 0x0f) == 8) {
            // Button A..H
            int b = data1 >> 4;

            switch (device->rotation()) {
            case LaunchpadDevice::Rotation0: {
                int mappedX = 0x40 + 0x10*(deviceIndex % 2);
                int mappedY = b + 8*(deviceIndex / 2);
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation90: {
                int mappedX = 0x60 + 8*(deviceIndex % 2) + (7-b) + 8*(deviceIndex / 2);
                int mappedY = 0;
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation180: {
                int mappedX = 0x40 + 0x10*(deviceIndex % 2);
                int mappedY = (7-b) + 8*(deviceIndex / 2);
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation270: {
                int mappedX = 0x60 + 8*(deviceIndex % 2) + b + 8*(deviceIndex / 2);
                int mappedY = 0;
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            }
        } else if ((data1 & 0x0f) <= 7) {
            // grid button
            int x = data1 & 0x07;
            int y = data1 >> 4;

            switch (device->rotation()) {
            case LaunchpadDevice::Rotation0: {
                int mappedX = 8*(deviceIndex % 2) + x;
                int mappedY = 8*(deviceIndex / 2) + y;
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation90: {
                int mappedX = 8*(deviceIndex / 2) + (7-y);
                int mappedY = 8*(deviceIndex % 2) + x;
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation180: {
                int mappedX = 8*(deviceIndex % 2) + (7-x);
                int mappedY = 8*(deviceIndex / 2) + (7-y);
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation270: {
                int mappedX = 8*(deviceIndex / 2) + y;
                int mappedY = 8*(deviceIndex % 2) + (7-x);
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            }
        }
    } else if (event == 0x0b && channel == 0) {
        if (data1 >= 0x68 && data1 <= 0x6f) {
            // Button 1..8
            int b = data1 & 0x07;

            switch (device->rotation()) {
            case LaunchpadDevice::Rotation0: {
                int mappedX = 0x60 + 8*(deviceIndex % 2) + b + 8*(deviceIndex / 2);
                int mappedY = 0;
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation90: {
                int mappedX = 0x40 + 0x10*(deviceIndex % 2);
                int mappedY = b + 8*(deviceIndex / 2);
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation180: {
                int mappedX = 0x60 + 8*(deviceIndex % 2) + (7-b) + 8*(deviceIndex / 2);
                int mappedY = 0;
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            case LaunchpadDevice::Rotation270: {
                int mappedX = 0x40 + 0x10*(deviceIndex % 2);
                int mappedY = (7-b) + 8*(deviceIndex / 2);
                _blm->sendNoteEvent(mappedY, mappedX, data2);
            } break;
            }
        }
    }
}

void LaunchpadController::clearLeds()
{
    for (const auto &device : _devices) {
        device->clearLeds();
    }
}

void LaunchpadController::setGridLed(int x, int y, int state)
{
    int col = x & 7;
    int row = y & 7;
    int velocity = LaunchpadDevice::stateToVelocity(state);

    int deviceIndex = -1;
    if (x <= 7 && y <= 7) {
        deviceIndex = 0;
    } else if (x <= 15 && y <= 7) {
        deviceIndex = 1;
    } else if (x <= 7 && y <= 15) {
        deviceIndex = 2;
    } else if (x <= 15 && y <= 15) {
        deviceIndex = 3;
    }

    if (deviceIndex >= 0 && deviceIndex < _count) {
        LaunchpadDevice *device = _devices[deviceIndex].get();
        switch (device->rotation()) {
        case LaunchpadDevice::Rotation0:
            device->sendMessage(MidiMessage(0x90, col + row*0x10, velocity));
            break;
        case LaunchpadDevice::Rotation90:
            device->sendMessage(MidiMessage(0x90, row + (7-col)*0x10, velocity));
            break;
        case LaunchpadDevice::Rotation180:
            device->sendMessage(MidiMessage(0x90, (7-col) + (7-row)*0x10, velocity));
            break;
        case LaunchpadDevice::Rotation270:
            device->sendMessage(MidiMessage(0x90, (7-row) + col*0x10, velocity));
            break;
        }
    }
}

void LaunchpadController::setExtraColumnLed(int x, int y, int state)
{
    int velocity = LaunchpadDevice::stateToVelocity(state);
    int index = y & 7;

    int deviceIndex = -1;
    if (x == 0 && y <= 7) {
        deviceIndex = 0;
    } else if (x == 1 && y <= 7) {
        deviceIndex = 1;
    } else if (x == 0 && y <= 15) {
        deviceIndex = 2;
    } else if (x == 1 && y <= 15) {
        deviceIndex = 3;
    }

    if (deviceIndex >= 0 && deviceIndex < _count) {
        LaunchpadDevice *device = _devices[deviceIndex].get();
        switch (device->rotation()) {
        case LaunchpadDevice::Rotation0:
            device->sendMessage(MidiMessage(0x90, 0x08 + index*0x10, velocity));
            break;
        case LaunchpadDevice::Rotation90:
            device->sendMessage(MidiMessage(0xb0, 0x68 + index, velocity));
            break;
        case LaunchpadDevice::Rotation180:
            device->sendMessage(MidiMessage(0x90, 0x08 + (7-index)*0x10, velocity));
            break;
        case LaunchpadDevice::Rotation270:
            device->sendMessage(MidiMessage(0xb0, 0x68 + (7 - index), velocity));
            break;
        }
    }
}

void LaunchpadController::setExtraRowLed(int x, int y, int state)
{
    unsigned index = x & 7;
    unsigned velocity = LaunchpadDevice::stateToVelocity(state);

    int deviceIndex = -1;
    if (x <= 7 && y == 0) {
        deviceIndex = 0;
    } else if (x <= 15 && y == 0) {
        deviceIndex = 1;
    } else if (x <= 7 && y == 1) {
        deviceIndex = 2;
    } else if (x <= 15 && y == 1) {
        deviceIndex = 3;
    }

    if (deviceIndex >= 0 && deviceIndex < _count) {
        LaunchpadDevice *device = _devices[deviceIndex].get();
        switch (device->rotation()) {
        case LaunchpadDevice::Rotation0:
            device->sendMessage(MidiMessage(0xb0, 0x68 + index, velocity));
            break;
        case LaunchpadDevice::Rotation90:
            device->sendMessage(MidiMessage(0x90, 0x08 + (7-index)*0x10, velocity));
            break;
        case LaunchpadDevice::Rotation180:
            device->sendMessage(MidiMessage(0xb0, 0x68 + (7-index), velocity));
            break;
        case LaunchpadDevice::Rotation270:
            device->sendMessage(MidiMessage(0x90, 0x08 + index*0x10, velocity));
            break;
        }
    }
}

void LaunchpadController::setShiftLed(int state)
{
}    
