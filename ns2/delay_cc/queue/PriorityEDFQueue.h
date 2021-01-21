#ifndef NS2_PRIORITYEDFQUEUE_H
#define NS2_PRIORITYEDFQUEUE_H


#include <delay_cc/queue/mixins/DropInfoMixin.h>
#include <queue/queue.h>
#include <queue>
#include <set>
#include <unordered_map>

class PriorityEDFQueue : public DropInfoMixin<Queue> {
public:
    PriorityEDFQueue();
    ~PriorityEDFQueue() override;

public:
    void enque(Packet * packet) override;

    Packet *deque() override;

protected:
    void delay_bind_init_all() override;
    int delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer) override;

private:
    void enqueue_delay_packet(Packet * packet);
    void enqueue_ordinary_packet(Packet * packet);
    void drop_delay_packet();

    void erase_delay_packet(Packet * packet);

    struct EDFComparator {
        bool operator()(Packet * rhs, Packet * lhs) const;
    };
    struct PriorityEDFComparator {
        bool operator()(Packet * rhs, Packet * lhs) const;
    };

private:
    int use_fifo_processing_order_;

    double last_deque_time;

    std::deque<Packet *> non_delay_pkts_;

    std::set<Packet *, PriorityEDFComparator> priority_queue_;
    std::set<std::pair<double, Packet *>> fifo_queue_;
    std::unordered_map<Packet *, double> packet_to_enque_time_;
    std::set<Packet *, EDFComparator> expiration_queue_;
};


#endif //NS2_PRIORITYEDFQUEUE_H
