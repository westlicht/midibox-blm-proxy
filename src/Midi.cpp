#include "Midi.h"

#include "RtMidi.h"

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

void Midi::update()
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
        // TODO std::cout << "Connected MIDI devices changed!" << std::endl;
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

        for (int i = 0; i < _inputPorts.size(); ++i) {
            for (auto device : _devices) {
                if (!device->_connected && device->_midiPort == _inputPorts[i]) {
                    device->_midiIn.setCallback(callback, device);
                    device->_midiIn.openPort(i);
                    device->_midiOut.openPort(i);
                    device->_connected = true;
                    device->connected();
                    break;
                }
            }
        }

    }
}

void Midi::addDevice(MidiDevice *device, const std::string &name)
{
    device->_midiPort = name;
    device->_connected = false;
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

