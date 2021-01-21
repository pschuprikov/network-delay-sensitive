#ifndef NS2_FIFOEDFQUEUE_H
#define NS2_FIFOEDFQUEUE_H

#include <delay_cc/queue/mixins/DropInfoMixin.h>
#include <queue.h>
#include <deque>
#include <memory>


class FIFOEDFQueue : public DropInfoMixin<Queue> {
public:
    FIFOEDFQueue();

    void enque(Packet *packet) override;

    Packet *deque() override;

private:
    static constexpr const int NUM_PRIORITY_LEVELS = 8;
    static constexpr const int HIGH_PRIORITY = 0;
    static constexpr const int LOW_PRIORITY = NUM_PRIORITY_LEVELS - 1;

    class SubQueue;
private:
    [[nodiscard]] auto get_total_length() const -> int;
    [[nodiscard]] auto get_total_byte_length() const -> int;
    [[nodiscard]] auto get_expected_delay(int priority) const -> double;
    [[nodiscard]] auto get_delay_priority(double remaining_slack) -> int;

    void drop_from_lowest_queue(bool on_enque);

private:
    double rtt_init;
    int num_enqueues_since_last_lowest;
    int num_enqueues_since_last_highest;
    std::array<std::unique_ptr<SubQueue>, NUM_PRIORITY_LEVELS> queues_;

};


#endif
