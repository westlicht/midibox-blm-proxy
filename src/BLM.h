#pragma once

#include "MidiDevice.h"

class BLM : public MidiDevice {
public:
    BLM();
    ~BLM();

    void connected() override;
    void disconnected() override;
    void handleMessage(const MidiMessage &msg) override;
};
