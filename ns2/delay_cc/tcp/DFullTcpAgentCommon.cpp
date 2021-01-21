#include "DFullTcpAgentCommon.h"
#include <fstream>
#include <iostream>

auto DFullTcpAgentCommon::pre_foutput(int seqno) -> bool {
    if (enable_early_termination_ == 0 || proxy_->get_deadline() == 0) {
        return true;
    }

    auto const cur_time_us = int(Scheduler::instance().clock() * 1e6);
    if (cur_time_us >= proxy_->get_expiration_time_us()) {
        if (proxy_->get_signal_on_empty() && proxy_->get_iss() != seqno) {
            proxy_->set_early_terminated(true);
            proxy_->do_bufferempty();
            proxy_->set_early_terminated(false);
            return false;
        }
    }

    return true;
}

void
DFullTcpAgentCommon::pre_advance_bytes(int nb) {
    auto const nominal_deadline = calc_nominal_deadline(nb);
    proxy_->set_deadline(nominal_deadline);

    if (delay_transport_) {
        delay_transport_->initialize(nb, nominal_deadline);
    }
}

auto DFullTcpAgentCommon::calc_nominal_deadline(int num_bytes) -> int {
    if (delay_assigner_) {
        auto deadline_in_sec = delay_assigner_->assign_delay(num_bytes);
        if (deadline_in_sec == std::numeric_limits<decltype(deadline_in_sec)>::infinity()) {
            return 0;
        } else {
            return static_cast<int>(deadline_in_sec * 1e6);
        }
    }
    return 0;
}



