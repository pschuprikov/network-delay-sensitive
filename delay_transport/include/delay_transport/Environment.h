#ifndef DELAY_TRANSPORT_ENVIRONMENT_H
#define DELAY_TRANSPORT_ENVIRONMENT_H

#include <memory>
#include <delay_transport/Timer.h>

namespace delay_transport {

struct Environment {
    virtual auto get_max_bytes_in_packet() const -> int = 0;

    virtual auto get_current_time() const -> double = 0;

    virtual auto create_timer() -> std::shared_ptr<Timer> = 0;

    virtual auto get_maximum_slack() const -> double = 0;

    virtual auto link_speed_in_gbps() const -> int = 0;

    Environment() = default;
    Environment(Environment const&) = delete;
    Environment const& operator=(Environment const&) = delete;

    virtual ~Environment() = default;
};

}

#endif //DELAY_TRANSPORT_ENVIRONMENT_H
