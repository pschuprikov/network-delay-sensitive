#include "ExpirationTrackingMixin.h"

template<class Base>
void ExpirationTrackingMixin<Base>::enque(Packet * p) {
    expiration_tracker_.on_packet_enters(p);
    Base::enque(p);
}

template<class Base>
template<class R> 
auto ExpirationTrackingMixin<Base>::remove_expired(R remover) const -> bool {
    auto packets_to_remove = expiration_tracker_.get_expired_packets();
    if (packets_to_remove.empty()) {
        return false;
    }

    for (auto packet : packets_to_remove) {
        remover(packet);
    }

    return true;
}

template<class Base>
void ExpirationTrackingMixin<Base>::drop(Packet * packet) {
    this->expiration_tracker_.on_packet_leaves(packet);
    Base::drop(packet);
}

template<class Base>
auto ExpirationTrackingMixin<Base>::deque() -> Packet * {
    auto packet = Base::deque();
    if (packet != nullptr) {
        this->expiration_tracker_.on_packet_leaves(packet);
    }
    return packet;
}
