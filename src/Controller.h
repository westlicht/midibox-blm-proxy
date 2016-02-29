#pragma once

class Controller {
public:
    virtual void clearLeds() = 0;
    virtual void setGridLed(int x, int y, int state) = 0;
    virtual void setExtraColumnLed(int x, int y, int state) = 0;
    virtual void setExtraRowLed(int x, int y, int state) = 0;
    virtual void setShiftLed(int state) = 0;
};
