#ifndef NS2_FILEDELAYASSIGNER_H
#define NS2_FILEDELAYASSIGNER_H


#include <string>
#include <map>
#include <deque>
#include "DelayAssigner.h"

class FileDelayAssigner : public DelayAssigner {
public:
    explicit FileDelayAssigner(std::string const& filename);

    double assign_delay(int msg_len) override;

private:
    std::map<int, double> deadlines_;
};


#endif //NS2_FILEDELAYASSIGNER_H
