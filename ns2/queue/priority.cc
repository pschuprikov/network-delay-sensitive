/*
 * Strict Priority Queueing (SP)
 *
 * Variables:
 * queue_num_: number of CoS queues
 * thresh_: ECN marking threshold
 * mean_pktsize_: configured mean packet size in bytes
 * marking_scheme_: Disable ECN (0), Per-queue ECN (1) and Per-port ECN (2)
 */

#include <link/delay.h>
#include "packet.h"
#include "priority.h"
#include "flags.h"

#include <algorithm>
#include <exception>

static class PriorityClass : public TclClass {
 public:
	PriorityClass() : TclClass("Queue/Priority") {}
	TclObject* create(int, const char*const*) override {
		return (new Priority);
	}
} class_priority;

int const Priority::MAX_QUEUE_NUM;

void Priority::enque(Packet* p)
{
    ensure_queue_bound();

    auto prio = get_priority(p);

	hdr_flags* hf = hdr_flags::access(p);
	int qlimBytes = qlim_ * mean_pktsize_;
    // 1<=queue_num_<=MAX_QUEUE_NUM

	//queue length exceeds the queue limit
	if(TotalByteLength() + hdr_cmn::access(p)->size() > qlimBytes)
	{
        handle_overflow(p);
        return;
	}

	//Enqueue packet
	q_[prio]->enque(p);

    //Enqueue ECN marking: Per-queue or Per-port
    if((marking_scheme_==ECNMode ::PER_QUEUE && q_[prio]->byteLength()>thresh_*mean_pktsize_)||
    (marking_scheme_==ECNMode ::PER_PORT && TotalByteLength()>thresh_*mean_pktsize_))
    {
        if (hf->ect()) //If this packet is ECN-capable
            hf->ce()=1;
    }
}

Packet* Priority::deque()
{
    if(TotalByteLength()>0)
	{
        //high->low: 0->7
	    for(int i=0;i<queue_num_;i++)
	    {
		    if(q_[i]->length()>0)
            {
			    Packet* p=q_[i]->deque();
		        return (p);
		    }
        }
    }

	return nullptr;
}

Priority::Priority() {
    queue_num_=MAX_QUEUE_NUM;
    thresh_=65;
    mean_pktsize_=1500;
    marking_scheme_=ECNMode::PER_PORT;

    //Bind variables
    bind("queue_num_", &queue_num_);
    bind("thresh_",&thresh_);
    bind("mean_pktsize_", &mean_pktsize_);
    bind("marking_scheme_", reinterpret_cast<int *>(&marking_scheme_));

    //Init queues
    for(int i=0;i<MAX_QUEUE_NUM;i++)
    {
        q_[i] = std::unique_ptr<PacketQueue>(new PacketQueue);
    }
}
 
void Priority::handle_overflow(Packet * packet) {
    drop(packet);
}

int Priority::TotalByteLength() {
    int bytelength=0;
    for(int i=0; i<MAX_QUEUE_NUM; i++)
        bytelength+=q_[i]->byteLength();
    return bytelength;
}

int Priority::get_priority(Packet *packet) const {
    return clamp_priority(hdr_ip::access(packet)->prio());
}


auto Priority::clamp_priority(int priority) const -> int {
    return std::max(0, std::min(priority, queue_num_ - 1));
}

void Priority::ensure_queue_bound() {
    if (queue_num_ < 1 || queue_num_ > MAX_QUEUE_NUM) {
        throw std::runtime_error("wrong queue num for Priority");
    }
}
