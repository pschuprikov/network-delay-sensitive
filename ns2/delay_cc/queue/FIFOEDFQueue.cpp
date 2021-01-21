#include <delay_cc/hdr_delay.h>
#include <delay_cc/Logger.h>
#include <delay_cc/queue/mixins/DropInfoMixin.hpp>

#include "FIFOEDFQueue.h"
namespace {

auto get_fifo_enqueue_log() {
    static Logger logger("fifo_enque");
    return logger.get();
}

auto get_fifo_deque_log() {
    static Logger logger("fifo_deque");
    return logger.get();
}

auto get_fifo_rtt_change_log() {
    static Logger logger("fifo_rtt_change");
    return logger.get();
}

}

static class FIFOEDFQueueClass : TclClass {
public:
    FIFOEDFQueueClass() : TclClass("Queue/FIFOEDF") { }
    TclObject * create(int, char const * const *) override {
        return new FIFOEDFQueue();
    }
} fifo_edf_queue_class;

class FIFOEDFQueue::SubQueue {
public:
    SubQueue();

public:
    void enque(Packet * packet);

    auto deque() -> Packet *;

    [[nodiscard]]
    auto get_length() const -> int;

    [[nodiscard]]
    auto get_byte_length() const -> int;

private:
    int current_byte_length_;
    std::deque<Packet *> queue_;
};

void FIFOEDFQueue::SubQueue::enque(Packet *packet) {
    queue_.push_back(packet);
    current_byte_length_ += hdr_cmn::access(packet)->size();
}

auto FIFOEDFQueue::SubQueue::deque() -> Packet * {
    if (queue_.empty()) {
        return nullptr;
    }
    auto const hol = queue_.front();
    queue_.pop_front();
    current_byte_length_ -= hdr_cmn::access(hol)->size();
    return hol;
}

auto FIFOEDFQueue::SubQueue::get_byte_length() const -> int {
    return current_byte_length_;
}

auto FIFOEDFQueue::SubQueue::get_length() const -> int {
    return queue_.size();
}

FIFOEDFQueue::SubQueue::SubQueue() : current_byte_length_{} {

}

void FIFOEDFQueue::enque(Packet *packet) {
    auto dh = hdr_delay::access(packet);
    if (dh->is_present()) {
        if (dh->expiration_time() == std::numeric_limits<double>::infinity() || dh->priority() == 0) {
            queues_[LOW_PRIORITY]->enque(packet);
        } else {
            auto const priority = get_delay_priority(dh->current_slack());
            get_fifo_enqueue_log()->info("{} {} {} {}",
                                         priority,
                                         dh->current_slack(),
                                         get_expected_delay(priority),
                                         dh->hop_count());
            queues_[priority]->enque(packet);
        }
    } else {
        queues_[HIGH_PRIORITY]->enque(packet);
    }

    if (get_total_length() > qlim_) {
        drop_from_lowest_queue(true);
    }
}

Packet *FIFOEDFQueue::deque() {
    for (auto i = 0; i < int(queues_.size()); i++) {
        while (queues_[i]->get_length() > 0) {
            auto hol = queues_[i]->deque();
            auto const dh = hdr_delay::access(hol);
            if (dh->is_present() && dh->is_expired()) {
                drop_with_reason(hol, delay_transport::DropReason::EXPIRED_DEQUE);
                continue;
            } else {
                get_fifo_deque_log()->info("{}", i);
                return hol;
            }
        }
    }
    return nullptr;
}

FIFOEDFQueue::FIFOEDFQueue()
    : rtt_init{0.00005}
    , num_enqueues_since_last_lowest{0}
    , num_enqueues_since_last_highest{0} {
    for (auto& q : queues_) {
        q = make_unique<SubQueue>();
    }
}

int FIFOEDFQueue::get_total_length() const {
    auto total_length = 0;
    for (auto const& q : queues_) {
        total_length += q->get_length();
    }
    return total_length;
}

auto FIFOEDFQueue::get_total_byte_length() const -> int {
    auto total_byte_length = 0;
    for (auto const& q : queues_) {
        total_byte_length += q->get_byte_length();
    }
    return total_byte_length;
}

auto FIFOEDFQueue::get_expected_delay(int priority) const -> double {
    auto delay = 0.0;
    for (auto i = 0; i <= static_cast<int>(priority); i++) {
        delay += queues_[i]->get_byte_length() * 8 / get_link_bandwidth();
    }
    return delay;
}

void FIFOEDFQueue::drop_from_lowest_queue(bool) {
    for (auto i = int(queues_.size()) - 1; i >= 0; i--) {
        if (queues_[i]->get_length() > 0) {
            auto const hol = queues_[i]->deque();
            drop_with_reason(hol, delay_transport::DropReason::OVERFLOW);
            return;
        }
    }
}

auto FIFOEDFQueue::get_delay_priority(double remaining_slack) -> int {
    auto rtt_pow = rtt_init;

    auto priority = HIGH_PRIORITY + 1;
    for (; priority < LOW_PRIORITY - 1; rtt_pow *= 2, priority++) {
        if (remaining_slack < rtt_pow) {
            break;
        }
    }

    if (priority == HIGH_PRIORITY + 1) {
        num_enqueues_since_last_highest = 0;
    } else {
        num_enqueues_since_last_highest++;
    }
    if (priority == LOW_PRIORITY - 1) {
        num_enqueues_since_last_lowest = 0;
    } else {
        num_enqueues_since_last_lowest++;
    }

    if (num_enqueues_since_last_highest > qlim_) {
        num_enqueues_since_last_highest = 0;
        rtt_init *= 2.0;
        get_fifo_rtt_change_log()->info("1 {}", rtt_init);
    }
    if (num_enqueues_since_last_lowest > qlim_) {
        num_enqueues_since_last_lowest = 0;
        rtt_init /= 2.0;
        get_fifo_rtt_change_log()->info("0 {}", rtt_init);
    }

    return priority;
}
