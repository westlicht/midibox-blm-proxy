#include "LaunchpadDevice.h"
#include "LaunchpadController.h"
#include "Debug.h"
#include "Settings.h"

#include "Midi.h"

LaunchpadDevice::LaunchpadDevice(LaunchpadController *controller, int index) :
    _controller(controller),
    _index(index),
    _rotation(Rotation0),
    _buttonPressed(128, false)
{
    std::string port = Settings::instance().json()["launchpad"]["port"].string_value();
    if (port.empty()) {
        throw Exception("Invalid Launchpad MIDI port '%s'", port);
    }
    Midi::addDevice(this, port);
}

LaunchpadDevice::~LaunchpadDevice()
{
    Midi::removeDevice(this);
}

void LaunchpadDevice::setRotation(Rotation rotation)
{
    _rotation = rotation;
}

void LaunchpadDevice::clearLeds()
{
    sendMessage(MidiMessage(0xb0, 0x00, 0x00));
}

void LaunchpadDevice::setGridLed(int x, int y, int state)
{
    sendMessage(MidiMessage::noteOn(0, y * 16 + x, stateToVelocity(state)));
}

int LaunchpadDevice::stateToVelocity(int state)
{
    switch (state) {
    case 1:  return 0x3c;
    case 2:  return 0x0f;
    case 3:  return 0x3f;
    default: return 0x00;
    }
}

LaunchpadDevice::Corner LaunchpadDevice::noteToCorner(int note)
{
    switch (note) {
    case 0:   return TopLeft;
    case 7:   return TopRight;
    case 112: return BottomLeft;
    case 119: return BottomRight;
    default:  return Invalid;
    }
}

LaunchpadDevice::Rotation LaunchpadDevice::computeRotation(Corner source, Corner target)
{
    return Rotation((int(target) - int(source) + 4) % 4);
}

void LaunchpadDevice::connected()
{
    clearLeds();
    _buttonPressed = std::vector<bool>(128, false);
    DBG("Launchpad %d connected", _index);
}

void LaunchpadDevice::disconnected()
{
    DBG("Launchpad %d disconnected", _index);
}

void LaunchpadDevice::handleMessage(const MidiMessage &msg)
{
    _controller->handleMessage(this, msg);

    if (msg.isNoteOn()) {
        _buttonPressed[msg.note()] = msg.velocity() != 0;
        if (msg.velocity() != 0 && _buttonPressed[0] && _buttonPressed[7] && _buttonPressed[112] && _buttonPressed[119]) {
            _controller->startCalibration();
        }
    }
}
