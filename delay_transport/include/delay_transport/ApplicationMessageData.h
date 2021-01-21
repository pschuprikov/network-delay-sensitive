#ifndef DELAY_TRANSPORT_APPLICATIONMESSAGEDATA_H
#define DELAY_TRANSPORT_APPLICATIONMESSAGEDATA_H

#include "Peer.h"
#include <memory>
#include <optional>

namespace delay_transport {
struct ApplicationMessageData {
    double creation_time;
    double slack;

    int length_in_bytes;
    int total_bytes_on_wire;

    std::shared_ptr<Peer> to_peer;
    std::shared_ptr<Peer> from_peer;

    double get_expiration_time() const { return creation_time + slack; }
};
}

#endif //DELAY_TRANSPORT_APPLICATIONMESSAGEDATA_H
