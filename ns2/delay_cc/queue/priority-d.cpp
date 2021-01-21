#include "priority-d.h"
#include <delay_cc/hdr_delay.h>
#include <delay_cc/queue/mixins/ExpirationTrackingMixin.hpp>

static class DPriorityClass : public TclClass {
 public:
    DPriorityClass() : TclClass("Queue/Priority/D") {}
    TclObject* create(int, const char*const*) override {
        return (new DPriority);
    }
} class_priority;

void DPriority::enque(Packet *p) {
    auto const prio = get_priority(p);
    auto const dh = hdr_delay::access(p);
    if (dh->is_present() && Scheduler::instance().clock() + calc_expected_delay(prio) > dh->expiration_time()) {
        drop(p);

        return;
    }

    super::enque(p);
}

int DPriority::get_priority(Packet *packet) const {
    auto const dh = hdr_delay::access(packet);

    if (dh->is_present()) {
        return clamp_priority(queue_num_ - 1 - int(dh->priority()));
    }

    return super::get_priority(packet);
}

auto DPriority::calc_expected_delay(int prio) const -> double {
    double bandwidth = get_link_bandwidth();
    return q_[prio]->byteLength() * 8 / (double) bandwidth;
}

auto DPriority::deque() -> Packet * {
    remove_expired_from_queue();
    return super::deque();
}

auto DPriority::remove_expired_from_queue(Packet * packet) -> bool {
    return remove_expired([this,packet] (Packet * rm_packet) {
        if (rm_packet != packet) { // TODO: consider moving it to Tracking
            auto const prio = get_priority(rm_packet);
            q_[prio]->remove(rm_packet);
            drop(rm_packet);
        }
    });
}

void DPriority::handle_overflow(Packet * packet) {
    if (remove_expired_from_queue(packet)) {
        super::enque(packet);
        return;
    }

    super::handle_overflow(packet);
}
