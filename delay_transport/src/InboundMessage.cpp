#include "InboundMessage.h"
#include "AnalysisMetadataImpl.h"
#include <cassert>
#include <delay_transport/Error.h>

namespace delay_transport {

InboundMessage::InboundMessage(DelayPacketMetadata const &rx_packet,
                               Environment const *environment)
      : msg_byte_len(rx_packet.message_len), from_peer{rx_packet.from_peer},
        to_peer{rx_packet.to_peer}, msg_id_at_sender(rx_packet.message_id),
        msg_creation_time(static_cast<AnalysisMetadataImpl *>(
                                  rx_packet.analysis_metadata.get())
                                  ->message_creation_time),
        environment{environment} {
    awaiting_data.emplace_back(1, rx_packet.message_len);
}

bool InboundMessage::append_packet_data(DelayPacketMetadata const &rx_packet) {
    if (msg_byte_len == 0) {
        return true;
    }

    for (auto it = begin(awaiting_data); it != end(awaiting_data);) {
        if (it->first > rx_packet.last_byte ||
            it->second < rx_packet.first_byte) {
            it++;
            continue;
        }
        if (it->first < rx_packet.first_byte) {
            awaiting_data.emplace(it, it->first, rx_packet.first_byte - 1);
        }
        if (it->second > rx_packet.last_byte) {
            awaiting_data.emplace(it, rx_packet.last_byte + 1, it->second);
        }
        auto cur = it++;
        awaiting_data.erase(cur);
    }
    return awaiting_data.empty();
}

int InboundMessage::get_num_remaining_bytes() const {
    int result = 0;
    for (auto const &seg : awaiting_data) {
        result += seg.second - seg.first + 1;
    }
    return result;
}

} // namespace delay_transport
