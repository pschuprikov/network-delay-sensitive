#include "OutboundMessage.h"
#include "AnalysisMetadataImpl.h"
#include <delay_transport/Error.h>
#include <delay_transport/PacketPropertiesAssigner.h>
#include <cmath>
#include <cassert>
#include <numeric>

namespace delay_transport {

OutboundMessage::OutboundMessage(Environment *environment, ApplicationMessageData data, int msg_id,
                                 ExpirationListener *expiration_listener,
                                 std::unique_ptr<PacketPropertiesAssigner> packet_properties)
        : msg_id{msg_id}
        , app_message{std::move(data)}
        , environment{environment}
        , expiration_listener{expiration_listener}
        , packet_properties{std::move(packet_properties)}
        , has_expired{false}
        {
}

DelayPacketMetadata OutboundMessage::create_packet_metadata(int first_byte, int last_byte, int bytes_remaining) {
    check_expiration(bytes_remaining);
    return generate_packet_metadata(first_byte, last_byte, bytes_remaining);
}

double OutboundMessage::calc_expiration_time([[maybe_unused]] int bytes_remaining) const {
    if (has_expired) {
        return std::numeric_limits<ExpirationTime>::infinity();
    }

    return app_message.get_expiration_time();
}

DelayPacketMetadata
OutboundMessage::generate_packet_metadata(int first_byte, int last_byte, int bytes_remaining) const {
    auto packet_metadata = DelayPacketMetadata{};
    packet_metadata.to_peer = app_message.to_peer;
    packet_metadata.from_peer = app_message.from_peer;

    packet_metadata.first_byte = first_byte;
    packet_metadata.last_byte = last_byte;


    packet_metadata.message_id = msg_id;
    packet_metadata.message_len = app_message.length_in_bytes;

    auto analysis_metadata = std::make_shared<AnalysisMetadataImpl>();

    analysis_metadata->message_creation_time = app_message.creation_time;
    analysis_metadata->message_expiration_time = app_message.get_expiration_time();

    packet_metadata.analysis_metadata = std::move(analysis_metadata);

    packet_metadata.expiration_time = calc_expiration_time(bytes_remaining);

    packet_metadata.set_priority(calc_priority(bytes_remaining));
    set_same_priority(packet_metadata);

    return packet_metadata;
}

void OutboundMessage::handle_drop(
        [[maybe_unused]] const DelayPacketMetadata& packet, 
        [[maybe_unused]] const DropInfo &info) {
}

    auto OutboundMessage::to_expiration_time(std::pair<double, double> const &time_slack) -> double {
    return time_slack.first + time_slack.second;
}

double OutboundMessage::calc_priority(int bytes_remaining) const {
    return has_expired 
        || app_message.get_expiration_time() == std::numeric_limits<double>::infinity()
        || !packet_properties
        ? 0.0 : packet_properties->assign_value(bytes_remaining);
}

void OutboundMessage::check_expiration(int bytes_remaining) {
    // TODO: fix by removing magic constant
    if (!has_expired && environment->get_current_time() >= calc_expiration_time(bytes_remaining)) {
        expiration_listener->report_expiration(app_message, bytes_remaining);
        has_expired = true;
    }
}

void OutboundMessage::set_hop_based_priority(DelayPacketMetadata &packet) const {
    std::fill(begin(packet.priorities) + 1, end(packet.priorities), 1.0 / packet.priorities.size());
    std::partial_sum(begin(packet.priorities), end(packet.priorities), begin(packet.priorities));
}

void OutboundMessage::set_same_priority(DelayPacketMetadata &packet) const {
    std::fill(begin(packet.priorities), end(packet.priorities), packet.get_priority());
}

bool OutboundMessage::TimeSlackComparator::operator()(std::pair<double, double> const &rhs,
                                                      std::pair<double, double> const &lhs) const {
    return to_expiration_time(rhs) < to_expiration_time(lhs);
}
}
