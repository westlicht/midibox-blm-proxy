#pragma once

#include "MidiMessage.h"

#include <RtMidi/RtMidi.h>

#include <string>
#include <thread>
#include <algorithm>
#include <mutex>

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

    void sendMessageBuffered(const MidiMessage &msg)
    {
        if (_midiOut.isPortOpen()) {
            std::lock_guard<std::mutex> lock(_outputBufferMutex);
            std::copy(msg.data().begin(), msg.data().end(), std::back_inserter(_outputBuffer));
        }
    }

private:
    std::string _midiPort;
    RtMidiIn _midiIn;
    RtMidiOut _midiOut;
    bool _connected = false;

    std::vector<uint8_t> _outputBuffer;
    std::mutex _outputBufferMutex;

    friend class Midi;
};
