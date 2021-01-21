#ifndef __HOMATRANSPORT_DELAYTRANSPORT_H_
#define __HOMATRANSPORT_DELAYTRANSPORT_H_

#include <delay_transport/ApplicationMessageData.h>
#include <delay_transport/DelayTransport.h>
#include <delay_transport/Environment.h>

#include "Analyzer.h"
#include "InboundMessage.h"
#include "OutboundMessage.h"

#include <list>
#include <memory>
#include <unordered_map>

namespace delay_transport {

class DelayTransportImpl : public DelayTransport {
  public:
    explicit DelayTransportImpl(
            Environment *environment,
            std::shared_ptr<PacketPropertiesAssignerFactory> packet_properties);

    ~DelayTransportImpl() override;

    auto on_new_app_message(ApplicationMessageData const &msg)
            -> message_id_t override;

    void on_message_delivered(message_id_t delivered_msg_id) override;

    void on_packet_drop(DelayPacketMetadata packet, DropInfo info) override;

    void on_packet_received(const DelayPacketMetadata &packet,
                            const ReceptionInfo &info) override;

    DelayPacketMetadata create_packet_metadata(message_id_t msg_id,
                                               int first_byte, int last_byte,
                                               int bytes_remaining) override;

  private:
    Analyzer *get_analyzer();

  private:
    InboundMessage *find_inbound_message(
            DelayPacketMetadata const &packet,
            std::list<std::unique_ptr<InboundMessage>> const &rxMsgList) const;

  private:
    Environment *const environment;
    std::shared_ptr<PacketPropertiesAssignerFactory> const packet_properties;

    message_id_t next_msg_id;

    std::unordered_map<message_id_t, std::list<std::unique_ptr<InboundMessage>>>
            incomplete_msgs;
    std::list<std::unique_ptr<OutboundMessage>> outbound_msgs;

    static std::list<DelayTransportImpl *> instances;
};

} // namespace delay_transport

#endif
