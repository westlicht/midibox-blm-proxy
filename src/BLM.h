#pragma once

#include "MidiDevice.h"
#include "Controller.h"

class BLM : public MidiDevice {
public:
    BLM();
    ~BLM();

    void connected() override;
    void disconnected() override;
    void handleMessage(const MidiMessage &msg) override;

    void setController(Controller *controller);

    int buttonState(int col, int row) const;
    void setButtonState(int col, int row, int state);

    void setLed(int col, int row, int bit, int enabled);
    void setLedPattern8_H(int colOffset, int row, int bit, int pattern);
    void setLedPattern8_V(int col, int rowOffset, int bit, int pattern);

    void handleBlmMessage(const MidiMessage &msg);

    void sendLayout();
    void sendAck();

    void dump();

private:
    static const int MaxRows = 16+1;
    static const int MaxRowsExtraOffset = 16;
    static const int MaxCols = 16+2;
    static const int MaxColsExtraOffset = 16;

    Controller *_controller;

    int _colors;
    int _rows;
    int _cols;
    int _buttonState[MaxCols][MaxRows];
};
