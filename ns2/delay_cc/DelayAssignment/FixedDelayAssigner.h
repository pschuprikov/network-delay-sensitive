#ifndef NS2_FIXEDDELAYASSIGNER_H
#define NS2_FIXEDDELAYASSIGNER_H


#include "DelayAssigner.h"
#include <optional>

class FixedDelayAssigner : public DelayAssigner {
public:
    explicit FixedDelayAssigner(double delay, std::optional<int> size_upper_bound = std::nullopt);
public:
    double assign_delay(int msg_len) override;
private:
    double const delay_;
    std::optional<int> size_upper_bound_;
};


#endif //NS2_FIXEDDELAYASSIGNER_H
