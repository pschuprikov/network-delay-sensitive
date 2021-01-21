#ifndef NS2_EXPONENTIALDELAYASSIGNER_H
#define NS2_EXPONENTIALDELAYASSIGNER_H


#include <tools/ranvar.h>
#include "DelayAssigner.h"
#include <optional>

class ExponentialDelayAssigner : public DelayAssigner {
public:
    ExponentialDelayAssigner(std::optional<int> upper_bound_size,
                             double average, bool use_capping,
                             TopologyOracle *oracle);

    double assign_delay(int msg_len) override;

private:
    ExponentialRandomVariable rvar_;
    std::optional<int> const upper_bound_size_;
    bool const use_capping_;
    TopologyOracle * const oracle_;
};


#endif //NS2_EXPONENTIALDELAYASSIGNER_H
