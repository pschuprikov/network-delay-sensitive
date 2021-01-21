#ifndef NS2_AGENTINFOPROVIDER_H
#define NS2_AGENTINFOPROVIDER_H

struct AgentInfoProvider {
    virtual int get_num_buffered_bytes() = 0;

    virtual int get_max_bytes_in_packet() const = 0;

    virtual int link_speed_in_gbps() const = 0;

    AgentInfoProvider() = default;
    AgentInfoProvider(AgentInfoProvider const&) = delete;
    AgentInfoProvider const& operator=(AgentInfoProvider const&) = delete;

    virtual ~AgentInfoProvider() = default;
};

#endif //NS2_AGENTINFOPROVIDER_H
