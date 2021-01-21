#include "FixedDelayAssigner.h"

double FixedDelayAssigner::assign_delay(int msg_len) {
    if (size_upper_bound_ && *size_upper_bound_ < msg_len) {
        return std::numeric_limits<double>::infinity();
    } else {
        return delay_;
    }
}

FixedDelayAssigner::FixedDelayAssigner(double delay, std::optional<int> size_upper_bound)
    : delay_{delay}, size_upper_bound_{size_upper_bound} {

}
