#include "ExponentialDelayAssigner.h"
#include <iostream>

double ExponentialDelayAssigner::assign_delay(int msg_len) {
    if (!upper_bound_size_ || msg_len <= *upper_bound_size_) {
        auto const val = rvar_.value();
        if (use_capping_) {
            auto const lower_bound = 1.25 * oracle_->get_oracle_fct(msg_len);
            return max(val, lower_bound);
        } else {
            return val;
        }
    } else {
        return std::numeric_limits<double>::infinity();
    }
}

ExponentialDelayAssigner::ExponentialDelayAssigner(std::optional<int> upper_bound_size,
                                                   double average,
                                                   bool use_capping,
                                                   TopologyOracle *oracle)
    : rvar_{average}
    , upper_bound_size_{upper_bound_size}
    , use_capping_{use_capping}
    , oracle_{oracle} {
}
