#pragma once

#include "MidiDevice.h"

#include <vector>

class LaunchpadController;

class LaunchpadDevice : public MidiDevice {
public:
    enum Corner {
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft,
        Invalid,
    };

    enum Rotation {
        Rotation0,
        Rotation90,
        Rotation180,
        Rotation270,
    };

    LaunchpadDevice(LaunchpadController *controller, int index);
    ~LaunchpadDevice();

    int index() const { return _index; }

    Rotation rotation() const { return _rotation; }
    void setRotation(Rotation rotation);

    void clearLeds();
    void setGridLed(int x, int y, int state);

    static int stateToVelocity(int state);
    static Corner noteToCorner(int note);
    static Rotation computeRotation(Corner source, Corner target);


    // MidiDevice
    void connected() override;
    void disconnected() override;
    void handleMessage(const MidiMessage &msg) override;

private:
    LaunchpadController *_controller;
    int _index;
    Rotation _rotation;

    std::vector<bool> _buttonPressed;
};
