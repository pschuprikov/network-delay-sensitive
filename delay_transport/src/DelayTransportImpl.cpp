#include <utility>

#include "AnalysisMetadataImpl.h"
#include "DelayTransportImpl.h"
#include <delay_transport/Error.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <optional>
#include <random>

namespace delay_transport {

std::list<DelayTransportImpl *> DelayTransportImpl::instances;

DelayTransportImpl::DelayTransportImpl(
        Environment *environment,
        std::shared_ptr<PacketPropertiesAssignerFactory> packet_properties)
      : environment{environment},
        packet_properties{std::move(packet_properties)},
        next_msg_id(0) {
    static std::mt19937_64 merceneRand{};
    std::uniform_int_distribution<message_id_t> dist(
            0, std::numeric_limits<message_id_t>::max());
    next_msg_id = dist(merceneRand);
    instances.push_back(this);
}

DelayTransportImpl::~DelayTransportImpl() { instances.remove(this); }

auto DelayTransportImpl::on_new_app_message(ApplicationMessageData const &msg)
        -> message_id_t {
    outbound_msgs.push_back(std::make_unique<OutboundMessage>(
            environment, msg, next_msg_id, get_analyzer(),
            packet_properties ? packet_properties->create_instance(msg)
                              : nullptr));
    get_analyzer()->on_new_message(msg);
    return next_msg_id++;
}

void DelayTransportImpl::on_packet_received(
        const DelayPacketMetadata &packet,
        [[maybe_unused]] const ReceptionInfo &info) {
    auto &ex_message_list = incomplete_msgs[packet.message_id];

    auto *inbound_msg = find_inbound_message(packet, ex_message_list);

    if (inbound_msg == nullptr) {
        get_analyzer()->on_first_delivery(packet);
        ex_message_list.emplace_front(
                std::make_unique<InboundMessage>(packet, environment));
        inbound_msg = ex_message_list.front().get();
    }

    if (environment->get_current_time() >
        static_cast<AnalysisMetadataImpl *>(packet.analysis_metadata.get())
                ->message_creation_time) {
        get_analyzer()->on_message_timeout(
                packet.message_len, inbound_msg->get_num_remaining_bytes());
    }

    if (inbound_msg->append_packet_data(packet)) {
        auto iter = std::find_if(begin(ex_message_list), end(ex_message_list),
                                 [inbound_msg](auto const &x) {
                                     return x.get() == inbound_msg;
                                 });
        auto inbound_message_pointer = std::move(*iter);
        ex_message_list.erase(iter);

        get_analyzer()->on_receiver_delivery(packet.message_len);

        if (ex_message_list.empty()) {
            incomplete_msgs.erase(next_msg_id);
        }
    }
}

InboundMessage *DelayTransportImpl::find_inbound_message(
        DelayPacketMetadata const &packet,
        std::list<std::unique_ptr<InboundMessage>> const &rxMsgList) const {
    InboundMessage *inboundMsg = nullptr;
    for (std::unique_ptr<InboundMessage> const &curInboundMsg : rxMsgList) {
        if (curInboundMsg->from_peer->equal(*packet.from_peer)) {
            inboundMsg = curInboundMsg.get();
        }
    }
    return inboundMsg;
}

void DelayTransportImpl::on_packet_drop(DelayPacketMetadata packet,
                                        DropInfo info) {
    get_analyzer()->on_drop(packet, info);
    auto const msg_it =
            std::find_if(begin(outbound_msgs), end(outbound_msgs),
                         [&packet](auto const &msg) {
                             return msg->get_msg_id() == packet.message_id;
                         });

    if (msg_it != end(outbound_msgs)) {
        (*msg_it)->handle_drop(packet, info);
    }
}

void DelayTransportImpl::on_message_delivered(
        DelayTransport::message_id_t delivered_msg_id) {
    auto msg_it = std::find_if(begin(outbound_msgs), end(outbound_msgs),
                               [delivered_msg_id](auto const &x) {
                                   return x->get_msg_id() == delivered_msg_id;
                               });
    get_analyzer()->on_sender_delivery((*msg_it)->get_app_message());
    outbound_msgs.erase(msg_it);
}

Analyzer *DelayTransportImpl::get_analyzer() {
    static std::unique_ptr<Analyzer> analyzer;
    if (!analyzer) {
        analyzer = std::make_unique<Analyzer>(environment, 1.0, 0.01);
    }
    return analyzer.get();
}

DelayPacketMetadata
DelayTransportImpl::create_packet_metadata(message_id_t msg_id, int first_byte,
                                           int last_byte, int bytes_remaining) {
    auto const &msg_ptr = *std::find_if(
            begin(outbound_msgs), end(outbound_msgs),
            [msg_id](auto const &x) { return x->get_msg_id() == msg_id; });
    assert(first_byte > 0);
    assert(last_byte <= msg_ptr->get_app_message().length_in_bytes);
    auto const metadata = msg_ptr->create_packet_metadata(first_byte, last_byte,
                                                          bytes_remaining);
    get_analyzer()->on_packet_sent(metadata);
    return metadata;
}
} // namespace delay_transport
