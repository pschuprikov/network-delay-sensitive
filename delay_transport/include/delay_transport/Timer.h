#ifndef DELAY_TRANSPORT_TIMER_H
#define DELAY_TRANSPORT_TIMER_H

#include <functional>

namespace delay_transport {

struct Timer {
    virtual bool is_scheduled() const = 0;

    virtual void reschedule(double time, std::function<void()> callback) = 0;

    Timer() = default;

    Timer(Timer const &) = delete;

    Timer const &operator=(Timer const &) = delete;

    virtual ~Timer() = default;
};

}

#endif //DELAY_TRANSPORT_TIMER_H
