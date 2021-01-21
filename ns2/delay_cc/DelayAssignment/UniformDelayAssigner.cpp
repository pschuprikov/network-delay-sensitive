#include "UniformDelayAssigner.h"

double UniformDelayAssigner::assign_delay([[maybe_unused]] int msg_len) {
    return rvar_.value();
}

UniformDelayAssigner::UniformDelayAssigner(double min_value, double max_value)
    : rvar_{min_value, max_value} {
}
