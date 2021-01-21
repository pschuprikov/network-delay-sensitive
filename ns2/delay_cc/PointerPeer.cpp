#include "PointerPeer.h"

PointerPeer::PointerPeer(DelayTransport *transport) : transport{transport} {

}

auto PointerPeer::get_transport() const -> DelayTransport * {
    return transport;
}

auto PointerPeer::equal(delay_transport::Peer const &) const -> bool {
    // in NS2 there is only a single peer per agent
    return true;
}
