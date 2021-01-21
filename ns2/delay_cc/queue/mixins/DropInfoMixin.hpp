#include "DropInfoMixin.h"

template<class Base>
void DropInfoMixin<Base>::queue_drop_delay_packet(
        Packet *packet, 
        delay_transport::DropReason drop_reason) {
    auto *const dh = hdr_delay::access(packet);
    assert(dh->is_present());
    dh->transport()->on_packet_drop(packet, drop_reason);

    Base::drop(packet);
}

template<class Base>
void DropInfoMixin<Base>::drop_with_reason(
        Packet * packet, 
        delay_transport::DropReason drop_reason
        ) {
    on_drop(packet);
    auto const dh = hdr_delay::access(packet);
    if (dh->is_present()) {
        queue_drop_delay_packet(packet, drop_reason);
    } else {
        Base::drop(packet);
    }
}
