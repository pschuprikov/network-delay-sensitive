#ifndef DELAY_TRANSPORT_INBOUNDMESSAGE_H
#define DELAY_TRANSPORT_INBOUNDMESSAGE_H

#include <delay_transport/DelayPacketMetadata.h>
#include <delay_transport/Environment.h>
#include <list>

namespace delay_transport {

class InboundMessage {
  public:
    InboundMessage(DelayPacketMetadata const &rx_packet,
                   Environment const *environment);

    bool append_packet_data(DelayPacketMetadata const &rx_packet);
    int get_num_remaining_bytes() const;

  public:
    uint32_t msg_byte_len;
    std::shared_ptr<Peer> from_peer;
    std::shared_ptr<Peer> to_peer;
    uint64_t msg_id_at_sender;
    double msg_creation_time;

    Environment const *environment;
    std::list<std::pair<int, int>> awaiting_data;
};

} // namespace delay_transport

#endif // DELAY_TRANSPORT_INBOUNDMESSAGE_H
