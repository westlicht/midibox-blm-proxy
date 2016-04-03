#pragma once

#include "MidiDevice.h"

#include <string>

//! MIDI system.
class Midi {
public:
    //! Updates the MIDI system.
    //! Checks if registered MIDI devices are connected/disconnected.
    //! Needs to be called periodically.
    static void update();

    //! Registers a MIDI device.
    static void addDevice(MidiDevice *device, const std::string &name);
    //! Unregisters a MIDI device.
    static void removeDevice(MidiDevice *device);

    //! Returns a list of all detected MIDI input ports.
    static const std::vector<std::string> &inputPorts() { return _inputPorts; }
    //! Returns a list of all detected MIDI output ports.
    static const std::vector<std::string> &outputPorts() { return _outputPorts; }

private:
    static void checkDevices();

    static std::vector<std::string> _inputPorts;
    static std::vector<std::string> _outputPorts;
    static std::vector<MidiDevice *> _devices;
};
