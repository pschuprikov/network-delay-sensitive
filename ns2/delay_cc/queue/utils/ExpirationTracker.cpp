#include "ExpirationTracker.h"
#include "delay_cc/hdr_delay.h"
#include <scheduler.h>

#include <algorithm>

ExpirationTracker::ExpirationTracker() 
    : packets(comparator) {

}

void ExpirationTracker::on_packet_enters(Packet * packet) {
    auto const dh = hdr_delay::access(packet);
    if (dh->is_present()) {
        packets.insert(packet);
    }
}

void ExpirationTracker::on_packet_leaves(Packet * packet) {
    auto const dh = hdr_delay::access(packet);
    if (dh->is_present()) {
        packets.erase(packet);
    }
}

auto ExpirationTracker::get_expired_packets() const -> std::vector<Packet *> {
    auto const now = Scheduler::instance().clock();
    auto first_non_expired = std::find_if(
            begin(packets), end(packets), [now] (Packet * packet) {
                auto const dh = hdr_delay::access(packet);
                return dh->expiration_time() > now;
            }
        );
    return std::vector<Packet *>(begin(packets), first_non_expired);
}
