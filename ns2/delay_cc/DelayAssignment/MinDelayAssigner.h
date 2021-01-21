#ifndef NS2_MINDELAYASSIGNER_H
#define NS2_MINDELAYASSIGNER_H

#include "DelayAssigner.h"

#include <vector>
#include <memory>

class MinDelayAssigner : public DelayAssigner {
public:
    explicit MinDelayAssigner(std::vector<std::unique_ptr<DelayAssigner>> sub_assigners);
    double assign_delay(int msg_len) override;
private:
    std::vector<std::unique_ptr<DelayAssigner>> sub_assigners_;
};

#endif //NS2_MINDELAYASSIGNER_H
