#include "MinDelayAssigner.h"

#include <limits>

double MinDelayAssigner::assign_delay(int msg_len) {
    auto result = std::numeric_limits<double>::infinity();
    for (auto const& assigner : sub_assigners_) {
        result = std::min(result, assigner->assign_delay(msg_len));
    }
    return result;
}

MinDelayAssigner::MinDelayAssigner(std::vector<std::unique_ptr<DelayAssigner>> sub_assigners)
    : sub_assigners_{move(sub_assigners)} {
}
