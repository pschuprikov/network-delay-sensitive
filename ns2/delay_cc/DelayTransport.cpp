#include <tclcl.h>
#include <common/simulator.h>
#include <fstream>
#include "CommandDispatchHelper.h"
#include "TimerAdapter.h"
#include "DelayTransport.h"
#include "PointerPeer.h"
#include "hdr_delay.h"
#include "delay_cc/DelayAssignment/FileDelayAssigner.h"
#include "delay_cc/DelayAssignment/PIASDelayAssigner.h"
#include "delay_cc/DelayAssignment/FixedDelayAssigner.h"

namespace {

auto const MICROSECOND = 1.e-6;

}

DelayTransport::DelayTransport(AgentInfoProvider * agent_info)
    : agent_info_{agent_info}
    , transport_{}
    , maximum_slack_{0.0} {
}

auto DelayTransport::extract_metadata(Packet *packet) -> delay_transport::DelayPacketMetadata {
    delay_transport::DelayPacketMetadata packet_data;
    auto dh = hdr_delay::access(packet);
    assert(dh->is_present());

    packet_data.expiration_time = dh->expiration_time();
    packet_data.analysis_metadata = std::move(dh->analysis_meta());
    packet_data.first_byte = dh->first_byte();
    packet_data.last_byte = dh->last_byte();
    packet_data.message_id = dh->message_id();
    packet_data.message_len = dh->message_length();
    packet_data.from_peer = std::make_shared<PointerPeer>(dh->transport());
    packet_data.to_peer = std::make_shared<PointerPeer>(this);
    packet_data.retransmission_idx = dh->retransmission_idx();
    std::copy(begin(dh->priorities()), end(dh->priorities()), begin(packet_data.priorities));

    return packet_data;
}

void DelayTransport::initialize(int num_bytes, int nominal_deadline) {
    last_message_id_ = get_transport().on_new_app_message(
            create_app_data(num_bytes, nominal_deadline));
}


auto DelayTransport::create_app_data(int num_bytes, int nominal_deadline)
        -> delay_transport::ApplicationMessageData {
    delay_transport::ApplicationMessageData app_data;
    app_data.length_in_bytes = num_bytes;
    app_data.creation_time = Scheduler::instance().clock();
    app_data.from_peer = std::make_shared<PointerPeer>(this);

    app_data.slack = nominal_deadline == 0 ? std::numeric_limits<double>::infinity() : nominal_deadline * MICROSECOND;

    return app_data;
}

DelayTransport::~DelayTransport() = default;

void DelayTransport::on_packet_drop(Packet *packet, delay_transport::DropReason drop_reason) {
    delay_transport::DropInfo info {
        drop_reason,
        hdr_delay::access(packet)->hop_count(),
        hdr_delay::access(packet)->last_slack(),
        -1.0
    };
    get_transport().on_packet_drop(extract_metadata(packet), info);
}

auto DelayTransport::get_current_time() const -> double {
    return Scheduler::instance().clock();
}

auto DelayTransport::create_timer() -> shared_ptr<delay_transport::Timer> {
    return make_shared<TimerAdapter>();
}

auto DelayTransport::get_maximum_slack() const -> double {
    return maximum_slack_;
}

auto DelayTransport::get_max_bytes_in_packet() const -> int {
    return agent_info_->get_max_bytes_in_packet();
}

auto DelayTransport::get_num_buffered_bytes() -> int {
    return agent_info_->get_num_buffered_bytes();
}

auto DelayTransport::link_speed_in_gbps() const -> int {
    return agent_info_->link_speed_in_gbps();
}

auto DelayTransport::command(int argc, const char *const *argv) -> int {
    return nfp::command::dispatch(this, argc, argv)
        .add("set-packet-properties-assigner", &DelayTransport::set_packet_properties_assigner_command)
        .add("set-maximum-slack", &DelayTransport::set_maximum_slack_command)
        .result([] ([[maybe_unused]] auto argc, [[maybe_unused]] auto argv) { 
            return TCL_ERROR; 
        });
}

auto DelayTransport::set_packet_properties_assigner_command(std::string config_file) 
        -> nfp::command::TclResult {
    if (transport_) {
        throw std::logic_error("It's too late to set packet property assigner!");
    }

    auto const json = [&config_file] () {
        nlohmann::json json;
        std::ifstream(config_file) >> json;
        return json;
    } ();
    packet_properties_ = delay_transport::PacketPropertiesAssignerFactory::create_from_json(json, this);
    transport_ = delay_transport::DelayTransport::create_instance(this, packet_properties_);

    return nfp::command::TclResult::OK;
}

auto DelayTransport::set_maximum_slack_command(std::string const& slack) 
        -> nfp::command::TclResult {
    maximum_slack_ = stod(slack);

    return nfp::command::TclResult::OK;
}

void DelayTransport::on_recv(Packet *packet) {
    auto dh = hdr_delay::access(packet);
    if (dh->is_present()) {
        get_transport().on_packet_received(extract_metadata(packet), delay_transport::ReceptionInfo{dh->hop_count()});
    }
}

void DelayTransport::on_buffer_empty() {
    get_transport().on_message_delivered(last_message_id_);
}

void DelayTransport::on_packet_send(Packet *packet, int first_byte, int last_byte) {
    auto const packet_metadata = get_transport().create_packet_metadata(
            last_message_id_, first_byte, last_byte, get_num_buffered_bytes());

    auto const dh = hdr_delay::access(packet);
    dh->analysis_meta() = packet_metadata.analysis_metadata;
    dh->expiration_time() = packet_metadata.expiration_time;
    dh->message_id() = packet_metadata.message_id;
    dh->message_length() = packet_metadata.message_len;
    dh->first_byte() = packet_metadata.first_byte;
    dh->last_byte() = packet_metadata.last_byte;
    dh->retransmission_idx() = packet_metadata.retransmission_idx;
    std::copy(begin(packet_metadata.priorities), end(packet_metadata.priorities), 
              begin(dh->priorities()));
    dh->transport() = this;
}

auto DelayTransport::get_transport() -> delay_transport::DelayTransport& {
    if (!transport_) {
        transport_ = delay_transport::DelayTransport::create_instance(
                this, packet_properties_);
    }
    return *transport_;
}
