#ifndef NS2_EXPIRATIONTIMECONTROLLER_H
#define NS2_EXPIRATIONTIMECONTROLLER_H

#include <delay_cc/hdr_delay.h>

enum class AdmissionDecision {
    ADMIT, DROP, MARK
};

class DropTail;

struct ExpirationTimeController {
    static auto create(DropTail * queue) -> std::unique_ptr<ExpirationTimeController>;

    ExpirationTimeController() = default;

    virtual AdmissionDecision
    should_admit(double expiration_time) = 0;

    virtual void
    do_control(Packet *packet) = 0;

    virtual void
    dequeue_hook() = 0;

    ExpirationTimeController(ExpirationTimeController const&) = delete;
    ExpirationTimeController& operator=(ExpirationTimeController const&) = delete;
    virtual ~ExpirationTimeController() = default;
};

#endif //NS2_EXPIRATIONTIMECONTROLLER_H
