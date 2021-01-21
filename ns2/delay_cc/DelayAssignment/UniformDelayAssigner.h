#ifndef NS2_UNIFORMDELAYASSIGNER_H
#define NS2_UNIFORMDELAYASSIGNER_H

#include <tools/ranvar.h>
#include <delay_cc/DelayAssignment/DelayAssigner.h>

class UniformDelayAssigner : public DelayAssigner {
public:
    UniformDelayAssigner(double min_value, double max_value);

    double assign_delay(int msg_len) override;
private:
    UniformRandomVariable rvar_;
};


#endif //NS2_UNIFORMDELAYASSIGNER_H
