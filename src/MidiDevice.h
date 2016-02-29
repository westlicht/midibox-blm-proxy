#pragma once

#include "MidiMessage.h"

#include <RtMidi/RtMidi.h>

#include <string>

class MidiDevice {
public:
    virtual void connected() = 0;
    virtual void disconnected() = 0;
    virtual void handleMessage(const MidiMessage &msg) = 0;

    bool isConnected() const { return _connected; }

    void sendMessage(const MidiMessage &msg)
    {
        if (_midiOut.isPortOpen()) {
            auto data = msg.data();
            _midiOut.sendMessage(&data);
        }
    }

private:
    std::string _midiPort;
    RtMidiIn _midiIn;
    RtMidiOut _midiOut;
    bool _connected = false;

    friend class Midi;
};
