#ifndef DELAY_TRANSPORT_OUTBOUNDMESSAGE_H
#define DELAY_TRANSPORT_OUTBOUNDMESSAGE_H

#include "ExpirationListener.h"
#include <delay_transport/ApplicationMessageData.h>
#include <delay_transport/DelayPacketMetadata.h>
#include <delay_transport/DropInfo.h>
#include <delay_transport/Environment.h>
#include <delay_transport/PacketPropertiesAssigner.h>
#include <map>
#include <optional>
#include <set>

namespace delay_transport {

class OutboundMessage {
  public:
    OutboundMessage(
            Environment *environment, ApplicationMessageData data, int msg_id,
            ExpirationListener *expiration_listener,
            std::unique_ptr<PacketPropertiesAssigner> packet_properties);

    DelayPacketMetadata create_packet_metadata(int first_byte, int last_byte,
                                               int bytes_remaining);

    void handle_drop(const DelayPacketMetadata &packet, DropInfo const &info);

    [[nodiscard]] int get_msg_id() const { return msg_id; }

    [[nodiscard]] auto get_app_message() const
            -> ApplicationMessageData const & {
        return app_message;
    }

  private:
    struct TimeSlackComparator {
        bool operator()(std::pair<double, double> const &rhs,
                        std::pair<double, double> const &lhs) const;
    };

  private:
    static auto to_expiration_time(std::pair<double, double> const &time_slack)
            -> double;

  private:
    [[nodiscard]] auto generate_packet_metadata(int first_byte, int last_byte,
                                                int bytes_remaining) const
            -> DelayPacketMetadata;

    void set_hop_based_priority(DelayPacketMetadata &packet) const;

    void set_same_priority(DelayPacketMetadata &packet) const;

    void check_expiration(int bytes_remaining);

    [[nodiscard]] auto calc_expiration_time(int bytes_remaining) const
            -> double;

    [[nodiscard]] auto calc_priority(int bytes_remaining) const -> double;

  private:
    int const msg_id;
    ApplicationMessageData const app_message;
    Environment *const environment;
    ExpirationListener *const expiration_listener;
    std::unique_ptr<PacketPropertiesAssigner> const packet_properties;

    bool has_expired;
};

} // namespace delay_transport

#endif // DELAY_TRANSPORT_OUTBOUNDMESSAGE_H
