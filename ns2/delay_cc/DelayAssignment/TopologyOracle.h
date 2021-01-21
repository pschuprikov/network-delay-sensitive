#ifndef NS2_TOPOLOGYORACLE_H
#define NS2_TOPOLOGYORACLE_H

struct TopologyOracle {
    virtual double get_oracle_fct(int num_bytes) = 0;

    TopologyOracle() = default;
    TopologyOracle(TopologyOracle const&) = delete;
    TopologyOracle const& operator=(TopologyOracle const&) = delete;

    virtual ~TopologyOracle() = default;
};

#endif //NS2_TOPOLOGYORACLE_H
