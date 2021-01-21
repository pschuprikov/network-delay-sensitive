#include <common/scheduler.h>
#include <limits>
#include <algorithm>
#include "LastDelayController.h"

LastDelayController::LastDelayController()
    : last_delay_bytes_{-1}
    , min_above_last_delay_bytes_{-1}
    , last_delay_set_time_{0.0} {

}

void LastDelayController::check_expiration() {
    if (last_delay_bytes_ != -1 && Scheduler::instance().clock() > last_delay_set_time_ + 0.001) {
        last_delay_bytes_ = min_above_last_delay_bytes_;
        min_above_last_delay_bytes_ = -1;
        last_delay_set_time_ = Scheduler::instance().clock();
    }
}

void LastDelayController::do_control(double slack, int mean_pkt_size, int qlim_in_bytes,
                                     double link_bandwidth) {
    check_expiration();

    if (slack == std::numeric_limits<double>::infinity()) {
        return;
    }

    auto const cur_delay_bytes = int64_t(8.0 * slack * link_bandwidth);

    if (cur_delay_bytes <= mean_pkt_size || cur_delay_bytes >= qlim_in_bytes) {
        return;
    }

    if (last_delay_bytes_ == -1 || cur_delay_bytes <= last_delay_bytes_) {
        last_delay_bytes_ = cur_delay_bytes;
        min_above_last_delay_bytes_ = -1;
        last_delay_set_time_ = Scheduler::instance().clock();
    } else  if (last_delay_bytes_ != -1) {
        if (min_above_last_delay_bytes_ == -1 || min_above_last_delay_bytes_ > cur_delay_bytes) {
            min_above_last_delay_bytes_ = cur_delay_bytes;
        }
    }
}

int LastDelayController::get_queue_limit_in_bytes(int orig_qlim_in_bytes) const {
    if (last_delay_bytes_ == -1) {
        return orig_qlim_in_bytes;
    } else {
        return std::min(orig_qlim_in_bytes, last_delay_bytes_);
    }
}
