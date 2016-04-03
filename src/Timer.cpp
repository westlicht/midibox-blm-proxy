#include "Timer.h"

#include <chrono>
#include <algorithm>

int Timer::_nextId = 1;
std::vector<Timer::Item> Timer::_items;

Timer::~Timer()
{
    stopTimer();
}

int Timer::startTimer(int interval)
{
    return addTimer(this, interval);
}

void Timer::stopTimer(int id)
{
    removeTimer(this, id);
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
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
    last = now;

    for (auto &item : _items) {
        item.update(delta);
    }
}

int Timer::addTimer(Timer *timer, int interval)
{
    _items.emplace_back(timer, _nextId++, interval);
    return _items.back()._id;
}

void Timer::removeTimer(Timer *timer, int id)
{
    //_items.erase(_items.begin(), std::remove_if(_items.begin(), _items.end(), [&] (const Item &item) {
    _items.erase(std::remove_if(_items.begin(), _items.end(), [&] (const Item &item) {
        return timer == item._timer && (id == -1 || id == item._id);
    }), _items.end());
}

Timer::Item::Item(Timer *timer, int id, int interval) :
    _timer(timer),
    _id(id),
    _interval(interval),
    _timeLeft(interval)
{
}

void Timer::Item::update(int delta)
{
    _timeLeft -= delta;
    while (_timeLeft <= 0) {
        _timer->handleTimer(_id);
        _timeLeft += _interval;
    }
}
