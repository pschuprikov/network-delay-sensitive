#ifndef NS2_TIMERADAPTER_H
#define NS2_TIMERADAPTER_H

#include <delay_transport/Timer.h>
#include <common/timer-handler.h>

class TimerAdapter : private TimerHandler, public delay_transport::Timer {
public:
    [[nodiscard]] auto is_scheduled() const -> bool override;

    void reschedule(double time, std::function<void()> callback) override;

private:
    void expire(Event *event) override;

private:
    std::function<void()> callback_;
};


#endif //NS2_TIMERADAPTER_H
