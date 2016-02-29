#pragma once

#include "Controller.h"

class LaunchpadController : public Controller {
public:
    void clearLeds() override;
    void setGridLed(int x, int y, int state) override;
    void setExtraColumnLed(int x, int y, int state) override;
    void setExtraRowLed(int x, int y, int state) override;
    void setShiftLed(int state) override;
};
