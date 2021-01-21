#ifndef NS2_EXPIRATIONTIMECONTROLLERIMPL_H
#define NS2_EXPIRATIONTIMECONTROLLERIMPL_H

#include "ExpirationTimeController.h"

class DropTail;

class ExpirationTimeControllerImpl : public ExpirationTimeController {
public:
    explicit ExpirationTimeControllerImpl(DropTail * queue);

    AdmissionDecision should_admit(double expiration_time) override;
    void do_control(Packet *packet) override;
    void dequeue_hook() override;

private:
    void check_expiration();
    void reset_expiration();

    AdmissionDecision adaptive_threshold(double slack) const;

private:
    DropTail const * const queue_;

    double expiration_time_bound_;
    double expiration_time_bound_set_time_;
};


#endif //NS2_EXPIRATIONTIMECONTROLLERIMPL_H
