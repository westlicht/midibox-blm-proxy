#pragma once

#include <array>
#include <vector>
#include <cstdint>

#include "tinyformat.h"

class MidiMessage {
public:
    MidiMessage(int data0, int data1, int data2)
    {
        _data[0] = data0;
        _data[1] = data1;
        _data[2] = data2;
        _valid = true;
    }

    MidiMessage(const std::vector<uint8_t> &data)
    {
        if (data.size() == 3) {
            _data[0] = data[0];
            _data[1] = data[1];
            _data[2] = data[2];
            _valid = true;
        }
    }

    bool isValid() const { return _valid; }

    const std::array<uint8_t, 3> &data() const { return _data; }
    std::vector<uint8_t> dataAsVector() const
    {
        std::vector<uint8_t> data(3);
        data[0] = _data[0];
        data[1] = _data[1];
        data[2] = _data[2];
        return data;
    }
    uint8_t data0() const { return _data[0]; }
    uint8_t data1() const { return _data[1]; }
    uint8_t data2() const { return _data[2]; }

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
            os << tfm::format("MidiMessage[type=ControlChange,channel=%d,controller=%d,value=%d]", msg.channel(), msg.data1(), msg.data2());
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
    std::array<uint8_t, 3> _data;

};
