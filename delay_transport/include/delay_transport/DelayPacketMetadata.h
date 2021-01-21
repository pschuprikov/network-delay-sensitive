#ifndef DELAY_TRANSPORT_DELAYPACKETMETADATA_H
#define DELAY_TRANSPORT_DELAYPACKETMETADATA_H

#include <delay_transport/Peer.h>
#include <memory>
#include <array>
#include "AnalysisMetadata.h"

namespace delay_transport {

using ExpirationTime = double;
using Priority = double;

struct DelayPacketMetadata {
    std::shared_ptr<Peer> from_peer;
    std::shared_ptr<Peer> to_peer;
    int first_byte;
    int last_byte;

    int message_id;
    int message_len;

    ExpirationTime expiration_time;
    std::array<double, 4> priorities;

    std::shared_ptr<AnalysisMetadata> analysis_metadata;

    int retransmission_idx;

    [[nodiscard]] auto get_num_message_bytes() const -> int {
        return last_byte - first_byte + 1;
    }

    [[nodiscard]] Priority get_priority() const {
        return priorities.front();
    }

    void set_priority(Priority priority) {
        priorities.front() = priority;
    }

    [[nodiscard]] bool is_retransmission() const {
        return retransmission_idx > 0;
    }
};
}

#endif //DELAY_TRANSPORT_DELAYPACKETMETADATA_H
