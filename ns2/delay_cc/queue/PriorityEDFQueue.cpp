#include "PriorityEDFQueue.h"
#include "delay_cc/hdr_delay.h"
#include <link/delay.h>

static class PriorityQueueClass : public TclClass {
public:
    PriorityQueueClass() : TclClass("Queue/PriorityEDF") {}
    TclObject* create(int, const char * const *) {
        return new PriorityEDFQueue();
    }
} class_priqrity_queue;

bool PriorityEDFQueue::EDFComparator::operator()(Packet *p1, Packet *p2) const {
    auto const dh1 = hdr_delay::access(p1);
    auto const dh2 = hdr_delay::access(p2);
    return dh1->expiration_time() < dh2->expiration_time()
        || (dh1->expiration_time() == dh2->expiration_time() && p1 < p2);
}

bool PriorityEDFQueue::PriorityEDFComparator::operator()(Packet *rhs, Packet *lhs) const {
    auto const dh1 = hdr_delay::access(rhs);
    auto const dh2 = hdr_delay::access(lhs);
    return dh1->priority() > dh2->priority()
        || (dh1->priority() == dh2->priority() && dh1->expiration_time() < dh2->expiration_time())
        || (dh1->priority() == dh2->priority() && dh1->expiration_time() == dh2->expiration_time() && rhs < lhs);
}

Packet *PriorityEDFQueue::deque() {
    if (!non_delay_pkts_.empty()) {
        auto result = non_delay_pkts_.front();
        non_delay_pkts_.pop_front();
        last_deque_time = Scheduler::instance().clock();
        return result;
    }

    while (!expiration_queue_.empty()) {
        auto hol = *begin(expiration_queue_);
        auto const dh = hdr_delay::access(hol);

        if (!dh->is_expired()) {
            break;
        }

        erase_delay_packet(hol);
        drop_with_reason(hol, delay_transport::DropReason::EXPIRED_DEQUE);
    }

    if (priority_queue_.empty()) {
        return nullptr;
    }

    auto const packet = use_fifo_processing_order_ ? begin(fifo_queue_)->second : *begin(priority_queue_);

    erase_delay_packet(packet);
    hdr_delay::access(packet)->increment_hop_count();

    last_deque_time = Scheduler::instance().clock();

    return packet;
}

void PriorityEDFQueue::enque(Packet *packet) {
    if (expiration_queue_.empty()) {
        last_deque_time = -1.0;
    }

    auto const dh = hdr_delay::access(packet);

    if (dh->is_present()) {
        enqueue_delay_packet(packet);
    } else {
        enqueue_ordinary_packet(packet);
    }
}

void PriorityEDFQueue::drop_delay_packet() {
    assert(!priority_queue_.empty());
    auto const eol = *(--end(priority_queue_));
    erase_delay_packet(eol);
    drop_with_reason(eol, delay_transport::DropReason::OVERFLOW);
}

PriorityEDFQueue::~PriorityEDFQueue() = default;

PriorityEDFQueue::PriorityEDFQueue()
    : use_fifo_processing_order_{0}, last_deque_time{-1.0} {
}

void PriorityEDFQueue::enqueue_delay_packet(Packet * packet) {
    auto const dh = hdr_delay::access(packet);
    assert(dh->is_present());
    if (dh->is_expired()) {
        drop_with_reason(packet, delay_transport::DropReason::EXPIRED_ENQUE);
    } else {
        dh->update_last_slack();
        expiration_queue_.insert(packet);
        priority_queue_.insert(packet);
        fifo_queue_.emplace(Scheduler::instance().clock(), packet);
        packet_to_enque_time_[packet] = Scheduler::instance().clock();
        if (int(priority_queue_.size() + non_delay_pkts_.size()) > qlim_) {
            drop_delay_packet();
        }
    }
}

void PriorityEDFQueue::enqueue_ordinary_packet(Packet *packet) {
    if (int(priority_queue_.size() + non_delay_pkts_.size() + 1) > qlim_) {
        if (!priority_queue_.empty()) {
            drop_delay_packet();
            non_delay_pkts_.push_back(packet);
        } else {
            Connector::drop(packet);
        }
    } else {
        non_delay_pkts_.push_back(packet);
    }
}

void PriorityEDFQueue::erase_delay_packet(Packet *packet) {
    assert(priority_queue_.size() == expiration_queue_.size() && expiration_queue_.size() == fifo_queue_.size());
    priority_queue_.erase(packet);
    expiration_queue_.erase(packet);
    fifo_queue_.erase(make_pair(packet_to_enque_time_[packet], packet));
    packet_to_enque_time_.erase(packet);
    assert(priority_queue_.size() == expiration_queue_.size() && expiration_queue_.size() == fifo_queue_.size());
}

void PriorityEDFQueue::delay_bind_init_all() {
    delay_bind_init_one("use_fifo_processing_order_");
    NsObject::delay_bind_init_all();
}

int
PriorityEDFQueue::delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer) {
    if (delay_bind_bool(varName, localName, "use_fifo_processing_order_", &use_fifo_processing_order_, tracer))
        return TCL_OK;
    return NsObject::delay_bind_dispatch(varName, localName, tracer);
}
