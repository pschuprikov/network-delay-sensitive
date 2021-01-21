
#include <algorithm>
#include <delay_cc/hdr_delay.h>
#include <flags.h>
#include "LAQueue.h"

namespace {

auto get_priority(Packet * packet)  {
    auto const dh = hdr_delay::access(packet);
    return make_tuple(
            dh->priority(), 
            -dh->expiration_time(), 
            hdr_cmn::access(packet)->uid());
};

auto get_value(Packet * packet) {
    auto const dh = hdr_delay::access(packet);
    return dh->priority();
};

}

static class LAQueueClass : TclClass {
public:
    LAQueueClass() : TclClass("Queue/LA") { }
    TclObject * create(int, char const * const *) override {
        return new LAQueue();
    }
} la_queue_class;

LAQueue::LAQueue()
    : thresh_{0}
    , ecn_enable_{0}
    , drop_strategy_{DropStrategy::DROP_LARGEST_SLACK}
    , delay_packets{}
    , ordinary_packets{}
    , ordinary_packets_total_size{0} {
    bind_bool("ecn_enable_", &ecn_enable_);
    bind_bool("thresh_", &thresh_);
    bind("drop_strategy_", reinterpret_cast<int*>(&drop_strategy_));
}

Packet * LAQueue::deque() {
    Packet * result  = nullptr;
    if (!ordinary_packets.empty()) {
        result = ordinary_packets.back();
        ordinary_packets.pop_back();
        ordinary_packets_total_size -= hdr_cmn::access(result)->size();
    } else if (!delay_packets.empty()) {
        result = delay_packets.back();
        delay_packets.pop_back();
    }
    return result;
}

void LAQueue::enque(Packet * packet) {
    auto const dh = hdr_delay::access(packet);

    if (dh->is_present()) {
        enque_delay_packet(packet);
    } else {
        enque_ordinary_packet(packet);
    }

    if (int(length()) > qlim_) {
        drop_packet();
    }

    if (ecn_enable_ && is_ecn_threshold_reached()) {
        mark_ecn();
    }
}

void LAQueue::enque_ordinary_packet(Packet * packet) {
    ordinary_packets.push_front(packet);
    ordinary_packets_total_size += hdr_cmn::access(packet)->size();
    fix_delay();
}

void LAQueue::enque_delay_packet(Packet *packet) {
    insert_and_fix_delay(packet);
}

void LAQueue::insert_and_fix_delay(Packet *packet) {
    auto total_size = ordinary_packets_total_size;

    delay_packets.push_back(nullptr);

    auto out_it = rbegin(delay_packets);

    for (auto cur_it = out_it + 1; cur_it != rend(delay_packets); cur_it++) {
        if (packet && !check_alive_or_drop(packet, total_size, delay_transport::DropReason::EXPIRED_ENQUE)) {
            packet = nullptr;
        }

        if (packet && get_priority(*cur_it) < get_priority(packet)) {
            std::swap(packet, *cur_it);
        }

        if (check_alive_or_drop(*cur_it, total_size, delay_transport::DropReason::EXPIRED_ENQUE)) {
            total_size += hdr_cmn::access(*cur_it)->size();
            *(out_it++) = *cur_it;
        }
    }

    if (packet && check_alive_or_drop(packet, total_size, delay_transport::DropReason::EXPIRED_ENQUE)) {
        *(out_it++) = packet;
    }

    if (out_it == rbegin(delay_packets)) {
        delay_packets.clear();
    } else {
        delay_packets.erase(std::rotate(begin(delay_packets), out_it.base(), end(delay_packets)),
                            end(delay_packets));
    }
}

auto LAQueue::calc_expected_delay(int num_bytes) const -> double {
    return num_bytes * 8 / get_link_bandwidth();
}

auto LAQueue::will_expire_with_prefix(Packet *packet, int num_bytes_prefix) const -> bool {
    auto const dh = hdr_delay::access(packet);
    if (!dh->is_present()) {
        return false;
    }
    return dh->will_expire(calc_expected_delay(num_bytes_prefix + hdr_cmn::access(packet)->size()));
}

void LAQueue::fix_delay() {
    insert_and_fix_delay(nullptr);
}

auto LAQueue::check_alive_or_drop(Packet *packet, int num_bytes_prefix, delay_transport::DropReason reason) -> bool {
    if (will_expire_with_prefix(packet, num_bytes_prefix)) {
        drop_with_reason(packet, reason);
        return false;
    } else {
        return true;
    }
}

void LAQueue::drop_packet() {
    if (!delay_packets.empty()) {
        auto drop_it = get_delay_packet_to_drop();
        drop_with_reason(*drop_it, delay_transport::DropReason::OVERFLOW);
        delay_packets.erase(drop_it);
    } else if (!ordinary_packets.empty()) {
        ordinary_packets_total_size -= hdr_cmn::access(ordinary_packets.front())->size();
        drop_with_reason(ordinary_packets.front(), delay_transport::DropReason::OVERFLOW);
        ordinary_packets.pop_front();
    }
}

auto LAQueue::is_ecn_threshold_reached() const -> bool {
    return int(length()) >= thresh_;
}

auto LAQueue::length() const -> size_t {
    return delay_packets.size() + ordinary_packets.size();
}

void LAQueue::mark_ecn() {
    // TODO: don't mark ordinary packets
    // TODO: mark using get_delay_packet_to_drop
    auto const packet = !delay_packets.empty() ? delay_packets.front() : ordinary_packets.front();
    if (packet) {
        auto const hf = hdr_flags::access(packet);
        if (hf->ect()) {
            hf->ce() = 1;
        }
    }
}

auto LAQueue::get_delay_packet_to_drop() const -> vector<Packet *>::const_iterator {
    switch (drop_strategy_) {
        case DropStrategy ::DROP_SMALLEST_SLACK:
            return --find_if_not(begin(delay_packets), end(delay_packets), [this](Packet *packet) {
                return get_value(packet) == get_value(delay_packets.front());
            });
        case DropStrategy ::DROP_LARGEST_SLACK:
            return begin(delay_packets);
        default: abort();
    }
}

