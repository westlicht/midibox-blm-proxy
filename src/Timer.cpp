#include "Timer.h"

#include <chrono>
#include <algorithm>

std::vector<Timer *> Timer::_timers;

Timer::~Timer()
{
    stopTimer();
}

void Timer::startTimer(int intervalMs)
{
    _interval = intervalMs;
    addTimer(this);
}

void Timer::stopTimer()
{
    removeTimer(this);
}

void Timer::updateTimers()
{
    static bool initialized;
    static std::chrono::high_resolution_clock::time_point last;
    if (!initialized) {
        last = std::chrono::high_resolution_clock::now();
        initialized = true;
    }
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
    last = now;

    for (auto timer : _timers) {
        timer->_timeLeft -= duration;
        while (timer->_timeLeft <= 0) {
            timer->handleTimer();
            timer->_timeLeft += timer->_interval;
        }
    }
}

void Timer::addTimer(Timer *timer)
{
    timer->_timeLeft = timer->_interval;
    _timers.emplace_back(timer);
}

void Timer::removeTimer(Timer *timer)
{
    _timers.erase(std::remove(_timers.begin(), _timers.end(), timer), _timers.end());
}
