#pragma once

#include "MidiDevice.h"
#include "Controller.h"
#include "Timer.h"
#include "UdpSocket.h"

//! BLM class.
class BLM : public MidiDevice, public Timer {
public:
    //! BLM protocol.
    enum Protocol {
        MIDI,
        OSC,
    };

    //! BLM layout.
    enum Layout {
        Half, // 16x8
        Full, // 16x16
    };

    //! Constructor.
    BLM();
    //! Destructor.
    ~BLM();

    //! Returns the controller assigned to the BLM.
    Controller *controller() const { return _controller; }
    //! Sets the controller assigned to the BLM.
    void setController(Controller *controller);

    //! Called when the controller is connected.
    void controllerConnected(Layout layout);
    //! Called when the controller is disconnected.
    void controllerDisconnected();

    Layout layout() const { return _layout; }
    void setLayout(Layout layout);

    int buttonState(int col, int row) const;
    void setButtonState(int col, int row, int state);

    void sendNoteEvent(int channel, int note, int velocity);

    void dump();

    // MidiDevice callbacks.
    void connected() override;
    void disconnected() override;
    void handleMessage(const MidiMessage &msg) override;

    // Timer callback.
    void handleTimer(int id) override;

private:
    void setLed(int col, int row, int bit, int enabled);
    void setLedPattern8_H(int colOffset, int row, int bit, int pattern);
    void setLedPattern8_V(int col, int rowOffset, int bit, int pattern);

    void handleBlmMessage(const MidiMessage &msg);
    void handleOscInput();

    void sendButtonState(int col, int row, int state);
    void sendLayout();
    void sendAck();
    void sendMessageActiveProtocol(const MidiMessage &msg);

    void update();
    void updateIdlePattern();

    static const int MaxRows = 16+1;
    static const int MaxRowsExtraOffset = 16;
    static const int MaxCols = 16+2;
    static const int MaxColsExtraOffset = 16;

    enum State {
        WaitController,
        WaitBlm,
        Running,
    };

    Protocol _protocol;
    UdpSocket _socket;

    Controller *_controller;
    Layout _layout;

    State _state;

    bool _blmReady;
    bool _controllerReady;

    int _colors;
    int _rows;
    int _cols;
    int _buttonState[MaxCols][MaxRows];

    int _updateTimer = 0;
    int _idleTimer = 0;
    int _oscTimer = 0;

    int _idleCounter;
};
