#ifndef NS2_DELAYASSIGNER_H
#define NS2_DELAYASSIGNER_H

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "TopologyOracle.h"

struct DelayAssigner {
    static auto create_from_json(nlohmann::json const &json_filename, TopologyOracle * oracle) -> std::unique_ptr<DelayAssigner>;

    virtual double assign_delay(int msg_len) = 0;

    DelayAssigner() = default;
    DelayAssigner(DelayAssigner const&) = delete;
    DelayAssigner const& operator=(DelayAssigner const&) = delete;

    virtual ~DelayAssigner() = default;
};
#endif //NS2_DELAYASSIGNER_H
