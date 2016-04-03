#pragma once

#include <vector>

//! Timer.
class Timer {
public:
    virtual ~Timer();

    //! Starts a new timer with the given interval in millisconds.
    //! Returns a unique timer id.
    int startTimer(int interval);

    //! Stops the timer with a given id or all timers if id == -1.
    void stopTimer(int id = -1);

    //! Called when a timer expires.
    virtual void handleTimer(int id) = 0;

    //! Updates the timer system and calls handleTimer() for each expired timer.
    //! Needs to be called periodically!
    static void updateTimers();
private:
    class Item {
    public:
        Item(Timer *timer, int id, int interval);
        void update(int delta);
    private:
        Timer *_timer;
        int _id;
        int _interval;
        int _timeLeft;

        friend class Timer;
    };

    static int addTimer(Timer *timer, int interval);
    static void removeTimer(Timer *timer, int id);

    static int _nextId;
    static std::vector<Item> _items;
};
