#include "drop-tail-d.h"

#include <delay_cc/hdr_delay.h>
#include <delay_cc/queue/mixins/DropInfoMixin.hpp>
#include <delay_cc/queue/mixins/ExpirationTrackingMixin.hpp>
#include <flags.h>
#include <common/agent.h>
#include <classifier/classifier.h>
#include <delay_transport/DropInfo.h>
#include <optional>

static class DDropTailClass : public TclClass {
public:
    DDropTailClass() : TclClass("Queue/DropTail/D") {}
    auto create(int, const char*const*) -> TclObject* override {
        return (new DDropTail);
    }
} class_d_drop_tail;

auto DDropTail::get_expected_delay(Packet *packet) const -> double {
    switch (static_cast<DelayEstimationMode>(delay_estimation_mode_)) {
        case DelayEstimationMode ::LOCAL:
            return calc_local_expected_delay(packet);

        case DelayEstimationMode ::GLOBAL:
            return calc_global_expected_delay(packet);

        case DelayEstimationMode::MINIMAL:
            return calc_minimal_delay(packet);
    }

    abort();
}

auto DDropTail::calc_minimal_delay(Packet * packet) const -> double {
    return hdr_cmn::access(packet)->size() * 8 / (double) get_link_bandwidth();
}

auto DDropTail::calc_global_expected_delay(Packet *packet) const -> double {
    auto cur = target_;
    while (!dynamic_cast<DropTail*>(cur) && !dynamic_cast<Agent*>(cur)) {
        auto const mb_classifier = dynamic_cast<Classifier *>(cur);

        if (mb_classifier) {
            cur = mb_classifier->find(packet);
            continue;
        }

        auto const mb_connector = dynamic_cast<Connector *>(cur);
        if (mb_connector) {
            cur = mb_connector->target();
            continue;
        }

        abort();
    }
    auto const mb_drop_tail = dynamic_cast<DDropTail *>(cur);

    return std::max(calc_local_expected_delay(packet), mb_drop_tail ? mb_drop_tail->calc_global_expected_delay(packet) : 0.0);
}

auto DDropTail::calc_local_expected_delay(Packet * packet) const -> double {
    auto const byte_length = 
        get_queue()->byteLength() + hdr_cmn::access(packet)->size();
    return  byte_length * 8 / (double) get_link_bandwidth();
}


void DDropTail::enque(Packet * p) {
    if (hdr_delay::access(p)->is_present()) {
        pre_enque_delay_packet(p);

        auto const mb_drop = check_delay_packet_admission(p);

        if (mb_drop.has_value()) {
            pre_enque();
            drop_with_reason(p, *mb_drop);
            return;
        }
    }

    super::enque(p);
}

void DDropTail::pre_enque_delay_packet(Packet * packet) {
    auto const dh = hdr_delay::access(packet);
    if (last_delay_controller_enabled_) {
        last_delay_controller_.do_control(dh->current_slack(),
            get_mean_packet_size(), qlim_in_bytes(), get_link_bandwidth());
    }
    if (expiration_time_controller_enabled_) {
        expiration_time_controller_->do_control(packet);
    }
}

auto DDropTail::check_delay_packet_admission(Packet * packet) const
        -> std::optional<delay_transport::DropReason> {
    auto const dh = hdr_delay::access(packet);

    if (dh->will_expire(get_expected_delay(packet))) {
        return delay_transport::DropReason::EXPIRED_ENQUE;
    }

    if (expiration_time_controller_enabled_) {
        return check_admission_by_expiration_time_controller(packet);
    }

    return std::nullopt;
}

auto DDropTail::check_admission_by_expiration_time_controller(Packet * packet) const
        -> optional<delay_transport::DropReason> {
    auto const dh = hdr_delay::access(packet);

    switch (expiration_time_controller_->should_admit(dh->expiration_time())) {
        case AdmissionDecision ::ADMIT:
            return std::nullopt;

        case AdmissionDecision ::DROP:
            return delay_transport::DropReason::ADMISSION_POLICY;

        case AdmissionDecision ::MARK:
            auto const fh = hdr_flags::access(packet);
            if (fh->ect()) {
                fh->ce() = 1;
            } else {
                return delay_transport::DropReason::ADMISSION_POLICY;
            }
    }
    abort();
}

auto DDropTail::deque() -> Packet * {
    remove_expired_from_queue(delay_transport::DropReason::EXPIRED_DEQUE);

    auto const packet = super::deque();

    if (packet != nullptr) {
        auto const dh = hdr_delay::access(packet);
        if (dh->is_present()) {
            dh->increment_hop_count();
        }
    }

    expiration_time_controller_->dequeue_hook();
    return packet;
}

DDropTail::DDropTail()
        : delay_estimation_mode_{DelayEstimationMode::MINIMAL}
        , last_delay_controller_enabled_{0}
        , expiration_time_controller_enabled_{0}
        , expiration_time_controller_{} {
    expiration_time_controller_ = ExpirationTimeController::create(this);
    bind("delay_estimation_mode_", reinterpret_cast<int*>(&delay_estimation_mode_));
    bind_bool("last_delay_controller_enabled_", &last_delay_controller_enabled_);
    bind_bool("expiration_time_controller_enabled_", &expiration_time_controller_enabled_);
}

auto DDropTail::will_overflow(Packet * packet) const -> bool {
    if (get_queue_measured_in_bytes() && last_delay_controller_enabled_) {
        return get_queue()->byteLength() + hdr_cmn::access(packet)->size() 
            > last_delay_controller_.get_queue_limit_in_bytes(qlim_in_bytes());
    } else {
        return super::will_overflow(packet);
    }
}

void DDropTail::handle_overflow(Packet * packet) {
    auto packets_to_remove = std::vector<Packet *>{};

    if (remove_expired_from_queue(delay_transport::DropReason::EXPIRED_ENQUE)) {
        get_queue()->enque(packet);
        return;
    }

    super::handle_overflow(packet);
}

auto DDropTail::remove_expired_from_queue(delay_transport::DropReason reason) -> bool {
    return remove_expired([this,reason] (Packet * packet) {
        get_queue()->remove(packet);
        drop_with_reason(packet, reason);
    });
}
