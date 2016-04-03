#include "LaunchpadDevice.h"
#include "LaunchpadController.h"
#include "Debug.h"
#include "Settings.h"

#include "Midi.h"

LaunchpadDevice::LaunchpadDevice(LaunchpadController *controller, int index, const std::string &port, Rotation rotation) :
    _controller(controller),
    _index(index),
    _rotation(rotation),
    _buttonPressed(128, false)
{
    Midi::addDevice(this, port);
}

LaunchpadDevice::~LaunchpadDevice()
{
    Midi::removeDevice(this);
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

void LaunchpadDevice::connected()
{
    clearLeds();
    _buttonPressed = std::vector<bool>(128, false);
    _controller->deviceConnected(this);
}

void LaunchpadDevice::disconnected()
{
    _controller->deviceDisconnected(this);
}

void LaunchpadDevice::handleMessage(const MidiMessage &msg)
{
    _controller->deviceMessage(this, msg);
}
