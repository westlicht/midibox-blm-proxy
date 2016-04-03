#pragma once

class BLM;

//! Abstract interface for controllers.
class Controller {
public:
    void setBLM(BLM *blm) { _blm = blm; }

    virtual void clearLeds() = 0;
    virtual void setGridLed(int x, int y, int state) = 0;
    virtual void setExtraColumnLed(int x, int y, int state) = 0;
    virtual void setExtraRowLed(int x, int y, int state) = 0;
    virtual void setShiftLed(int state) = 0;

protected:
    BLM *_blm = nullptr;
};
