#pragma once

#include "MidiDevice.h"

#include <string>

class Midi {
public:
    static void update();
    static void shutdown();

    static void addDevice(MidiDevice *device, const std::string &name);
    static void removeDevice(MidiDevice *device);

    static const std::vector<std::string> &inputPorts() { return _inputPorts; }
    static const std::vector<std::string> &outputPorts() { return _outputPorts; }

private:
    static void checkDevices();

    static std::vector<std::string> _inputPorts;
    static std::vector<std::string> _outputPorts;
    static std::vector<MidiDevice *> _devices;
};
