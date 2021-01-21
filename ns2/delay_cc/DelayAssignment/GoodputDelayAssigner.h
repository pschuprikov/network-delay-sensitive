#ifndef NS2_GOODPUTDELAYASSIGNER_H
#define NS2_GOODPUTDELAYASSIGNER_H


#include <optional>
#include "DelayAssigner.h"

class GoodputDelayAssigner : public DelayAssigner {
public:
    explicit GoodputDelayAssigner(int64_t goodput, std::optional<int> size_lower_bound = std::nullopt);

    double assign_delay(int msg_len) override;

private:
    int64_t const goodput_;
    std::optional<int> size_lower_bound_;
};


#endif //NS2_GOODPUTDELAYASSIGNER_H
