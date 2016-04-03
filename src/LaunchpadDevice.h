#pragma once

#include "MidiDevice.h"

#include <vector>

class LaunchpadController;

//! Represents a single launchpad device.
class LaunchpadDevice : public MidiDevice {
public:
    enum Rotation {
        Rotation0 = 0,
        Rotation90 = 90,
        Rotation180 = 180,
        Rotation270 = 270,
    };

    //! Constructor.
    LaunchpadDevice(LaunchpadController *controller, int index, const std::string &port, Rotation rotation);
    //! Destructor.
    ~LaunchpadDevice();

    //! Returns the index assigned to this launchpad device instance.
    int index() const { return _index; }

    //! Returns the rotation.
    Rotation rotation() const { return _rotation; }

    //! Clears all leds.
    void clearLeds();
    //! Sets the state of a single grid led.
    void setGridLed(int x, int y, int state);

    //! Converts a button state to a velocity value (color).
    static int stateToVelocity(int state);

    // MidiDevice callbacks.
    void connected() override;
    void disconnected() override;
    void handleMessage(const MidiMessage &msg) override;

private:
    LaunchpadController *_controller;
    int _index;
    Rotation _rotation;

    std::vector<bool> _buttonPressed;
};
