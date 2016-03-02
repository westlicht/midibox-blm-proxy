#include "LaunchpadController.h"
#include "LaunchpadDevice.h"
#include "Debug.h"
#include "BLM.h"
#include "MidiMessage.h"

LaunchpadController::LaunchpadController() :
    _state(Normal)
{
    for (int i = 0; i < MaxDevices; ++i) {
        _devices.emplace_back(new LaunchpadDevice(this, i));
    }

    _devices[0]->setRotation(LaunchpadDevice::Rotation270);
}

LaunchpadController::~LaunchpadController()
{
}

void LaunchpadController::handleMessage(LaunchpadDevice *device, const MidiMessage &msg)
{
    DBG("Launchpad %d received %s", device->index(), msg);

    int deviceIndex = device->index();

    switch (_state) {
    case Normal: {
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

    } break;
    case Calibration: {
        if (msg.isNoteOn() && msg.velocity() != 0 && (msg.note() == 0 || msg.note() == 7 || msg.note() == 112 || msg.note() == 119)) {
            _calibrationData.emplace_back(deviceIndex, msg.note());
        }
        if (_calibrationData.size() >= 4) {
            clearLeds();
            _state = Normal;
        }

    } break;
    }

}

void LaunchpadController::startCalibration()
{
    DBG("Start calibration ...");
    _state = Calibration;
    _calibrationData.clear();
    clearLeds();
    for (const auto &device : _devices) {
        device->setGridLed(0, 0, 3);
        device->setGridLed(7, 0, 3);
        device->setGridLed(7, 7, 3);
        device->setGridLed(0, 7, 3);
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

    if (deviceIndex >= 0) {
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

    if (deviceIndex >= 0) {
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

    if (deviceIndex >= 0) {
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
