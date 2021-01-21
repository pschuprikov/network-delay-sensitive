#ifndef NS2_DELAYTRANSPORT_H
#define NS2_DELAYTRANSPORT_H

#include <memory>
#include <common/packet.h>
#include <tools/CommandDispatchHelper.h>
#include <delay_transport/DelayTransport.h>
#include <delay_transport/Environment.h>
#include <delay_transport/PacketPropertiesAssignerFactory.h>
#include <delay_cc/DelayAssignment/DelayAssigner.h>
#include <delay_cc/DelayTransport_fwd.h>
#include "AgentInfoProvider.h"

class DelayTransport : public delay_transport::Environment {
public:
    explicit DelayTransport(AgentInfoProvider * agent_info);

    void on_packet_drop(Packet * packet, delay_transport::DropReason drop_reason);

    void on_packet_send(Packet *packet, int first_byte, int last_byte);

    void on_recv(Packet * packet);

    void on_buffer_empty();

    void initialize(int num_bytes, int nominal_deadline);

    [[nodiscard]] 
    auto link_speed_in_gbps() const -> int override;

    [[nodiscard]] 
    auto get_max_bytes_in_packet() const -> int override;

    [[nodiscard]] 
    auto get_current_time() const -> double override;

    auto create_timer() -> std::shared_ptr<delay_transport::Timer> override;

    [[nodiscard]] 
    auto get_maximum_slack() const -> double override;

    auto command(int argc, const char *const *argv) -> int;

    ~DelayTransport() override;

private:
    auto extract_metadata(Packet *packet) -> delay_transport::DelayPacketMetadata;

    auto create_app_data(int num_bytes, int nominal_deadline) -> delay_transport::ApplicationMessageData;

private:
    auto get_num_buffered_bytes() -> int;

    auto set_packet_properties_assigner_command(std::string config_file) 
        -> nfp::command::TclResult;
    auto set_maximum_slack_command(std::string const& slack) 
        -> nfp::command::TclResult;

    auto get_transport() -> delay_transport::DelayTransport&;

private:

private:
    AgentInfoProvider * const agent_info_;

    std::shared_ptr<delay_transport::PacketPropertiesAssignerFactory> packet_properties_;
    std::unique_ptr<delay_transport::DelayTransport> transport_;

    delay_transport::DelayTransport::message_id_t  last_message_id_;

    double maximum_slack_;
};


#endif //NS2_DELAYTRANSPORT_H
