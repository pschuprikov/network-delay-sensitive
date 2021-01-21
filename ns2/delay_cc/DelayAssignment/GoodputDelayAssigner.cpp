#include "GoodputDelayAssigner.h"

double GoodputDelayAssigner::assign_delay(int msg_len) {
    if (size_lower_bound_ && msg_len < *size_lower_bound_) {
        return std::numeric_limits<double>::infinity();
    } else {
        return msg_len * 8 / (double) goodput_;
    }
}

GoodputDelayAssigner::GoodputDelayAssigner(int64_t goodput, std::optional<int> size_lower_bound)
    : goodput_{goodput}, size_lower_bound_{size_lower_bound} {
}
