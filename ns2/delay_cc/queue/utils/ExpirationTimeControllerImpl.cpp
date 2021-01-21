#include "ExpirationTimeControllerImpl.h"
#include <delay_transport/RawLogger.h>
#include <common/scheduler.h>
#include <delay_cc/hdr_delay.h>
#include <memory>
#include <queue/drop-tail.h>

namespace {

spdlog::logger * get_expiration_bound_log() {
    static delay_transport::RawLogger logger{"expiration_bound"};
    return logger.get();
}

}

auto ExpirationTimeController::create(DropTail *queue) -> std::unique_ptr<ExpirationTimeController> {
    return std::make_unique<ExpirationTimeControllerImpl>(queue);
}

ExpirationTimeControllerImpl::ExpirationTimeControllerImpl(DropTail * queue)
    : queue_{queue}
    , expiration_time_bound_{-1.0}
    , expiration_time_bound_set_time_{0.0} {

}

void ExpirationTimeControllerImpl::reset_expiration() {
    get_expiration_bound_log()->info(
            "{} {} {}",
            expiration_time_bound_,
            expiration_time_bound_set_time_,
            Scheduler::instance().clock()
    );
    expiration_time_bound_ = -1.0;
}

void ExpirationTimeControllerImpl::do_control(Packet *packet) {
    auto const dh = hdr_delay::access(packet);

    check_expiration();

    if (dh->expiration_time() == std::numeric_limits<double>::infinity()) {
        return;
    }

    if (expiration_time_bound_ < 0.0 || dh->expiration_time() < expiration_time_bound_) {
        expiration_time_bound_ = dh->expiration_time();
        expiration_time_bound_set_time_ = Scheduler::instance().clock();
    }

    if (dh->last_byte() == dh->message_length() && dh->expiration_time() == expiration_time_bound_) {
        reset_expiration();
    }
}

void ExpirationTimeControllerImpl::check_expiration() {
    if (expiration_time_bound_ < 0.0) {
        return;
    }
    if (Scheduler::instance().clock() > expiration_time_bound_) {
        reset_expiration();
    }
}

AdmissionDecision
ExpirationTimeControllerImpl::should_admit(double expiration_time) {
    auto const current_time = Scheduler::instance().clock();

    if (queue_->byteLength() > 0.6 * queue_->qlim_in_bytes()) { // DCTCP-like
        return AdmissionDecision ::MARK;
    }

    if (expiration_time_bound_ < 0.0 || expiration_time <= expiration_time_bound_) {
        return AdmissionDecision ::ADMIT;
    } else {
        //return AdmissionDecision ::MARK;
        return adaptive_threshold(expiration_time - current_time);
    }
}

void ExpirationTimeControllerImpl::dequeue_hook() {
    if (queue_->byteLength() == 0 && expiration_time_bound_ >= 0.0) {
        reset_expiration();
    }
}

AdmissionDecision ExpirationTimeControllerImpl::adaptive_threshold(double slack) const {
    auto const threshold = 0.6 * exp(-slack * 1e2) * queue_->qlim_in_bytes();

    if (queue_->byteLength() <= threshold) {
        return AdmissionDecision ::ADMIT;
    } else if (queue_->byteLength() <= threshold + (queue_->qlim_in_bytes() - threshold) / 2){
        return AdmissionDecision ::MARK;
    } else {
        return AdmissionDecision ::DROP;
    }
}
