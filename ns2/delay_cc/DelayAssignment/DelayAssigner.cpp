#include "DelayAssigner.h"
#include "FileDelayAssigner.h"
#include "FixedDelayAssigner.h"
#include "ExponentialDelayAssigner.h"
#include "TopologyOracle.h"
#include "MinDelayAssigner.h"
#include "GoodputDelayAssigner.h"
#include "UniformDelayAssigner.h"

#include <nlohmann/json.hpp>

#include <vector>

namespace {

auto create_from_json(nlohmann::json const& json, TopologyOracle * oracle) -> std::unique_ptr<DelayAssigner> {
    auto const& type = json.at("type");
    auto const& params = json.at("parameters");
    if (type == "file") {
        return std::make_unique<FileDelayAssigner>(params);
    } else if (type == "fixed") {
        return std::make_unique<FixedDelayAssigner>(
                params.at("delay"),
                params.count("size_upper_bound")
                ? std::optional(params.at("size_upper_bound").get<int>())
                : std::nullopt
        );
    } else if (type == "exponential") {
        return std::make_unique<ExponentialDelayAssigner>(
                params.count("size_upper_bound")
                ? std::optional(params.at("size_upper_bound").get<int>())
                : std::nullopt,
                params.at("average"),
                params.at("use_capping").get<bool>(),
                oracle
        );
    } else if (type == "min") {
        std::vector<std::unique_ptr<DelayAssigner>> sub_assigners;
        for (auto const &sub_assigner_json : params) {
            sub_assigners.push_back(create_from_json(sub_assigner_json, oracle));
        }
        return std::make_unique<MinDelayAssigner>(std::move(sub_assigners));
    } else if (type == "uniform") {
        return std::make_unique<UniformDelayAssigner>(params.at("min_delay"), params.at("max_delay"));
    } else if (type == "goodput") {
        return std::make_unique<GoodputDelayAssigner>(
                params.at("goodput"),
                params.count("size_lower_bound")
                ? std::optional(params.at("size_lower_bound").get<int>())
                : std::nullopt
                );
    } else {
        return nullptr;
    }
}

}

auto DelayAssigner::create_from_json(nlohmann::json const& json, TopologyOracle *oracle) -> std::unique_ptr<DelayAssigner> {
    return ::create_from_json(json, oracle);
}
