#include "LaunchpadDevice.h"
#include "LaunchpadController.h"

#include "Midi.h"

LaunchpadDevice::LaunchpadDevice(LaunchpadController *controller, int index) :
    _controller(controller),
    _index(index),
    _rotation(Rotation0)
{
    Midi::addDevice(this, "Launchpad");
}

LaunchpadDevice::~LaunchpadDevice()
{
    Midi::removeDevice(this);
}

void LaunchpadDevice::setRotation(Rotation rotation)
{
    _rotation = rotation;
}

void LaunchpadDevice::connected()
{
    tfm::printf("Launchpad %d connected!\n", _index);
}

void LaunchpadDevice::disconnected()
{
    tfm::printf("Launchpad %d disconnected!\n", _index);
}

void LaunchpadDevice::handleMessage(const MidiMessage &msg)
{
    _controller->handleMessage(this, msg);
}
