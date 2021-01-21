#include "hdr_delay.h"

int hdr_delay::offset_;

static class DelayHeaderClass : public PacketHeaderClass {
public:
    DelayHeaderClass() : PacketHeaderClass("PacketHeader/Delay", sizeof(hdr_delay)) {
        bind_offset(&hdr_delay::offset_);
    }

} class_delayhdr;

bool hdr_delay::is_expired() const {
    // TODO: what's the hell?
    return current_slack() + 1.e-5 <= 0.0;
}

double hdr_delay::current_slack() const {
    return expiration_time() - Scheduler::instance().clock();
}

void hdr_delay::update_last_slack() {
    last_slack_ = current_slack();
}

bool hdr_delay::will_expire(double expected_delay) const {
    return expected_delay > current_slack();
}

void hdr_delay::increment_hop_count() {
    hop_count_++;
}
