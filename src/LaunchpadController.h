#pragma once

#include "Controller.h"
#include "MidiMessage.h"
#include "LaunchpadDevice.h"

#include <vector>
#include <memory>

class LaunchpadController : public Controller {
public:
    LaunchpadController();
    ~LaunchpadController();

    void handleMessage(LaunchpadDevice *device, const MidiMessage &msg);
    void startCalibration();
    void finishCalibration();

    void clearLeds() override;
    void setGridLed(int x, int y, int state) override;
    void setExtraColumnLed(int x, int y, int state) override;
    void setExtraRowLed(int x, int y, int state) override;
    void setShiftLed(int state) override;

private:
    static const int MaxDevices = 4;

    std::vector<std::unique_ptr<LaunchpadDevice>> _devices;
    std::vector<int> _deviceMap;
    std::vector<int> _revDeviceMap;

    enum State {
        Normal,
        Calibration,
    };
    State _state;

    std::vector<std::pair<int, LaunchpadDevice::Corner>> _calibrationData;
};
