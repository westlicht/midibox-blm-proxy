#include "Midi.h"
#include "Debug.h"

#include <RtMidi/RtMidi.h>

std::vector<std::string> Midi::_inputPorts;
std::vector<std::string> Midi::_outputPorts;
std::vector<MidiDevice *> Midi::_devices;

static void callback(double timeStamp, std::vector<unsigned char> *message, void *user)
{
    if (message && user) {
        MidiDevice *device = static_cast<MidiDevice *>(user);
        device->handleMessage(MidiMessage(*message));
    }
}

static void errorCallback(RtMidiError::Type type, const std::string &errorText, void *user)
{
    DBG("RtMidi error: %s", errorText);
}

void Midi::update()
{
    checkDevices();
}

void Midi::shutdown()
{
    for (auto device : _devices) {
        if (device->_connected) {
            device->disconnected();
            device->_midiIn.closePort();
            device->_midiOut.closePort();
            device->_connected = false;
        }
    }
}

void Midi::addDevice(MidiDevice *device, const std::string &name)
{
    device->_midiPort = name;
    device->_connected = false;
    device->_midiIn.setErrorCallback(errorCallback, device);
    device->_midiOut.setErrorCallback(errorCallback, device);
    _devices.emplace_back(device);
}

void Midi::removeDevice(MidiDevice *device)
{
    if (device->_connected) {
        device->disconnected();
        device->_midiIn.closePort();
        device->_midiOut.closePort();
        device->_connected = false;
    }
    _devices.erase(std::remove(_devices.begin(), _devices.end(), device), _devices.end());
}

void Midi::checkDevices()
{
    std::vector<std::string> inputPorts;
    std::vector<std::string> outputPorts;

    RtMidiIn midiIn;
    for (int i = 0; i < midiIn.getPortCount(); ++i) {
        inputPorts.emplace_back(midiIn.getPortName(i));
    }
    RtMidiOut midiOut;
    for (int i = 0; i < midiOut.getPortCount(); ++i) {
        outputPorts.emplace_back(midiOut.getPortName(i));
    }

    if (inputPorts != _inputPorts || outputPorts != _outputPorts) {
        _inputPorts = inputPorts;
        _outputPorts = outputPorts;

        for (auto device : _devices) {
            if (device->_connected) {
                device->disconnected();
                device->_midiIn.cancelCallback();
                device->_midiIn.closePort();
                device->_midiOut.closePort();
                device->_connected = false;
            }
        }

        // TODO this assumes that input and output ports are in the same order!

        auto isMatchingPort = [] (const std::string &a, const std::string &b, const std::string &c) {
            return b.compare(0, a.size(), a) == 0 &&
                   c.compare(0, a.size(), a) == 0;
        };

        for (int i = 0; i < _inputPorts.size(); ++i) {
            for (auto device : _devices) {
                if (!device->_connected && isMatchingPort(device->_midiPort, _inputPorts[i], _outputPorts[i])) {
                    try {
                        device->_midiIn.openPort(i);
                        device->_midiIn.setCallback(callback, device);
                        device->_midiOut.openPort(i);
                        device->_connected = true;
                        device->connected();
                    } catch (const std::exception &e) {
                        device->_midiIn.cancelCallback();
                        device->_midiIn.closePort();
                        device->_midiOut.closePort();
                    }
                    break;
                }
            }
        }
    }
}
