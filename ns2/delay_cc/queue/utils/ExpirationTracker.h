#ifndef NS2_EXPIRATION_TRACKER_H
#define NS2_EXPIRATION_TRACKER_H

#include <delay_cc/hdr_delay.h>
#include <set>
#include <vector>

class ExpirationTracker {
public:
    ExpirationTracker();

    void on_packet_enters(Packet * packet);
    void on_packet_leaves(Packet * packet);

    auto get_expired_packets() const -> std::vector<Packet *>;

private:
    constexpr static auto const comp_key = [] (Packet * packet) {
        auto const dh = hdr_delay::access(packet);
        auto const ch = hdr_cmn::access(packet);
        return make_tuple(dh->expiration_time(), ch->uid());
    };

    constexpr static auto const comparator = [] (Packet * lhs, Packet * rhs) {
        return comp_key(lhs) < comp_key(rhs);
    };

private:
    std::set<Packet *, decltype(comparator)> packets;
};

#endif // EXPIRATION_TRACKER_H
