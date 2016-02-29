#include "BLM.h"

#include "Midi.h"

BLM::BLM()
{
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
    std::cout << "BLM input: " << msg << std::endl;
}

