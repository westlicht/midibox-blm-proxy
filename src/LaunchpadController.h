#pragma once

#include "Controller.h"
#include "MidiMessage.h"
#include "LaunchpadDevice.h"

#include <vector>
#include <memory>

//! Launchpad controller.
class LaunchpadController : public Controller {
public:
    //! Constructor.
    LaunchpadController();
    //! Destructor.
    ~LaunchpadController();

    //! Called when a launchpad device is connected.
    void deviceConnected(LaunchpadDevice *device);
    //! Called when a launchpad device is disconnected.
    void deviceDisconnected(LaunchpadDevice *device);
    //! Called when a launchpad device received a MIDI message.
    void deviceMessage(LaunchpadDevice *device, const MidiMessage &msg);

    // Controller methods.
    void clearLeds() override;
    void setGridLed(int x, int y, int state) override;
    void setExtraColumnLed(int x, int y, int state) override;
    void setExtraRowLed(int x, int y, int state) override;
    void setShiftLed(int state) override;

private:
    int _count;
    std::vector<std::unique_ptr<LaunchpadDevice>> _devices;
};
