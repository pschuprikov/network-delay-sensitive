#ifndef NS2_PRIORITY_D_H
#define NS2_PRIORITY_D_H


#include <delay_cc/queue/mixins/ExpirationTrackingMixin.h>
#include <common/packet.h>
#include <queue/priority.h>

class DPriority : public ExpirationTrackingMixin<Priority> {
    using super = ExpirationTrackingMixin;

public:
    void enque(Packet * p) override;

    auto deque() -> Packet * override;

protected:
    int get_priority(Packet * packet) const override;

    void handle_overflow(Packet * packet) override;

private:
    auto calc_expected_delay(int prio) const -> double;

    auto remove_expired_from_queue(Packet * packet = nullptr) -> bool;
};


#endif //NS2_PRIORITY_D_H
