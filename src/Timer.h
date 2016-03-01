#pragma once

#include <vector>

class Timer {
public:
    virtual ~Timer();

    void startTimer(int intervalMs);
    void stopTimer();

    virtual void handleTimer() = 0;

    static void updateTimers();
private:
    int _interval;
    int _timeLeft;

    static void addTimer(Timer *timer);
    static void removeTimer(Timer *timer);

    static std::vector<Timer *> _timers;
};
