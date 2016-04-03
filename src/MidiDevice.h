#pragma once

#include "MidiMessage.h"

#include <RtMidi/RtMidi.h>

#include <string>
#include <thread>
#include <algorithm>
#include <mutex>

//! MIDI device.
class MidiDevice {
public:
    //! Called when the MIDI device is connected.
    virtual void connected() = 0;
    //! Called when the MIDI device is disconnected.
    virtual void disconnected() = 0;
    //! Called when a MIDI message is received.
    virtual void handleMessage(const MidiMessage &msg) = 0;

    //! Returns true if the MIDI device is connected.
    bool isConnected() const { return _connected; }

    //! Sends a message.
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
