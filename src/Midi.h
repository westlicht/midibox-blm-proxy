#pragma once

#include "MidiDevice.h"

#include <string>

class Midi {
public:
    static void update();

    static void addDevice(MidiDevice *device, const std::string &name);
    static void removeDevice(MidiDevice *device);

private:
    static std::vector<std::string> _inputPorts;
    static std::vector<std::string> _outputPorts;
    static std::vector<MidiDevice *> _devices;
};
