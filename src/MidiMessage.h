#pragma once

#include <tinyformat/tinyformat.h>

#include <array>
#include <vector>
#include <cstdint>

class MidiMessage {
public:
    MidiMessage(int data0, int data1, int data2)
    {
        _data.emplace_back(data0);
        _data.emplace_back(data1);
        _data.emplace_back(data2);
        _valid = true;
    }

    MidiMessage(const std::vector<uint8_t> &data)
    {
        _data = data;
        _valid = true;
    }

    bool isValid() const { return _valid; }

    const std::vector<uint8_t> &data() const { return _data; }

    int event() const
    {
        return _data[0] >> 4;
    }

    int channel() const
    {
        return _data[0] & 0xf;
    }

    int note() const
    {
        return _data[1];
    }

    int velocity() const
    {
        return _data[2];
    }

    bool isSysex() const
    {
        return _data[0] == 0xf0;
    }

    bool isNoteOn() const
    {
        return (_data[0] & 0xf0) == 0x90;
    }

    bool isNoteOff() const
    {
        return (_data[0] & 0xf0) == 0x80;
    }

    bool isControlChange() const
    {
        return (_data[0] & 0xf0) == 0xb0;
    }

    static MidiMessage noteOn(int channel, int note, int velocity)
    {
        return MidiMessage(0x90 | channel, note, velocity);
    }

    static MidiMessage noteOff(int channel, int note, int velocity)
    {
        return MidiMessage(0x80 | channel, note, velocity);
    }

    static MidiMessage controlChange(int channel, int controller, int value)
    {
        return MidiMessage(0xb0 | channel, controller, value);
    }

    friend std::ostream &operator<<(std::ostream &os, const MidiMessage &msg) {
        if (!msg.isValid()) {
            os << "MidiMessage[invalid]";
        } else if (msg.isNoteOn()) {
            os << tfm::format("MidiMessage[type=NoteOn,channel=%d,note=%d,velocity=%d]", msg.channel(), msg.note(), msg.velocity());
        } else if (msg.isNoteOff()) {
            os << tfm::format("MidiMessage[type=NoteOff,channel=%d,note=%d,velocity=%d]", msg.channel(), msg.note(), msg.velocity());
        } else if (msg.isControlChange()) {
            os << tfm::format("MidiMessage[type=ControlChange,channel=%d,controller=%d,value=%d]", msg.channel(), msg.data()[1], msg.data()[2]);
        } else {
            os << tfm::format("MidiMessage[type=Unknown,channel=%d,data=", msg.channel());
            for (auto d : msg.data()) {
                os << tfm::format("%02x", d);
            }
            os << "]";
        }
        return os;
    }

private:
    bool _valid = false;
    std::vector<uint8_t> _data;
};
