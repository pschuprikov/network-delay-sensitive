#ifndef NS2_LAQUEUE_H
#define NS2_LAQUEUE_H

#include <deque>
#include <vector>

#include <queue/queue.h>
#include <delay_cc/queue/mixins/DropInfoMixin.h>

class LAQueue : public DropInfoMixin<Queue> {
public:
    enum class DropStrategy : int {
        DROP_SMALLEST_SLACK = 0,
        DROP_LARGEST_SLACK = 1
    };

public:
    LAQueue();
    ~LAQueue() override = default;

public:
    void enque(Packet * packet) override;

    Packet *deque() override;

    [[nodiscard]]
    auto length() const -> size_t;

private:
    using delay_packets_container = std::vector<Packet *>;

private:
    void enque_ordinary_packet(Packet * packet);

    void enque_delay_packet(Packet * packet);

    void drop_packet();

    [[nodiscard]]
    auto calc_expected_delay(int num_bytes) const -> double;

    auto will_expire_with_prefix(Packet * packet, int num_bytes_prefix) const -> bool;

    auto check_alive_or_drop(Packet *packet, int num_bytes_prefix, delay_transport::DropReason reason) -> bool;

    void insert_and_fix_delay(Packet *packet);

    void fix_delay();

    // TODO: unify with DropTail
    [[nodiscard]]
    auto is_ecn_threshold_reached() const -> bool;

    void mark_ecn();

    [[nodiscard]]
    auto get_delay_packet_to_drop() const -> delay_packets_container::const_iterator;

private:
    int thresh_;
    int ecn_enable_;
    DropStrategy drop_strategy_;

    delay_packets_container delay_packets;
    std::deque<Packet *> ordinary_packets;
    int ordinary_packets_total_size;

};

#endif //NS2_LAQUEUE_H
