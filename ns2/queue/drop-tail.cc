/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1994 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

[[maybe_unused]]
static const char rcsid[] =
    "@(#) $Header: /cvsroot/nsnam/ns-2/queue/drop-tail.cc,v 1.17 2004/10/28 23:35:37 haldar Exp $ (LBL)";

#include <common/agent.h>
#include <common/flags.h>
#include <classifier/classifier.h>
#include "drop-tail.h"
#include "queue.h"

namespace {

}

static class DropTailClass : public TclClass {
 public:
	DropTailClass() : TclClass("Queue/DropTail") {}
	TclObject* create(int, const char*const*) {
		return (new DropTail);
	}
} class_drop_tail;

void DropTail::reset()
{
	Queue::reset();
}

int 
DropTail::command(int argc, const char*const* argv) 
{
	if (argc==2) {
		if (strcmp(argv[1], "printstats") == 0) {
            print_summary_stats();
			return (TCL_OK);
		}
 		if (strcmp(argv[1], "shrink-queue") == 0) {
 			shrink_queue();
 			return (TCL_OK);
 		}
	}
	if (argc == 3) {
		if (!strcmp(argv[1], "packetqueue-attach")) {
			if (!(q_ = std::unique_ptr<PacketQueue>(dynamic_cast<PacketQueue*>(TclObject::lookup(argv[2])))))
				return (TCL_ERROR);
			else {
				pq_ = q_.get();
				return (TCL_OK);
			}
		}
	}
	return Queue::command(argc, argv);
}

/*
 * drop-tail
 */
void DropTail::enque(Packet* p)
{
    pre_enque();

    if (will_overflow(p)) {
	    handle_overflow(p);
	} else {
	    q_->enque(p);
	}

    if (ecn_enable_ == 1 && is_ecn_threshold_reached())	{
        mark_ecn();
    }
}

void DropTail::pre_enque() {
    if (summarystats) {
        updateStats(qib_ ? q_->byteLength() : q_->length());
    }
}

//AG if queue size changes, we drop excessive packets...
void DropTail::shrink_queue() 
{
        int qlimBytes = qlim_ * mean_pktsize_;
	if (debug_)
		printf("shrink-queue: time %5.2f qlen %d, qlim %d\n",
 			Scheduler::instance().clock(),
			q_->length(), qlim_);
        while ((!qib_ && q_->length() > qlim_) || 
            (qib_ && q_->byteLength() > qlimBytes)) {
                if (drop_front_) { /* remove from head of queue */
                        Packet *pp = q_->deque();
                        drop(pp);
                } else {
                        Packet *pp = q_->tail();
                        q_->remove(pp);
                        drop(pp);
                }
        }
}

Packet* DropTail::deque()
{
    if (summarystats) {
        Queue::updateStats(qib_ ? q_->byteLength() : q_->length());
    }

    Packet * packet = nullptr;
    if (deque_prio_) {
        packet = deque_priority();
	} else if (drop_smart_) {
        packet = deque_with_drop_smart();
    } else {
		packet = q_->deque();
	}

    return packet;
}

Packet *DropTail::deque_with_drop_smart() {
    Packet *p = q_->deque();
    if (p) {
        hdr_ip* h = hdr_ip::access(p);
        FlowKey fkey;
        fkey.src = h->saddr();
        fkey.dst = h->daddr();
        fkey.fid = h->flowid();

        char* fkey_buf = (char*) &fkey;
        int length = sizeof(fkey);
        std::string fkey_string(fkey_buf, length);

        std::hash<std::string> string_hasher;
        size_t signature = string_hasher(fkey_string);
        sq_queue_.push(signature);

        if (sq_counts_.find(signature) != sq_counts_.end()) {
            sq_counts_[signature]++;
        } else {
            sq_counts_[signature] = 1;
        }

        if (sq_queue_.size() > sq_limit_) {
            size_t temp = sq_queue_.front();
            sq_queue_.pop();
            sq_counts_[temp]--;
            if (sq_counts_[temp] == 0)
                sq_counts_.erase(temp);
        }
    }
    return p;
}

void DropTail::print_summary_stats() {
    printf("True average queue: %5.3f", true_ave_);
    if (qib_)
            printf(" (in bytes)");
    printf(" time: %5.3f\n", total_time_);
}

auto DropTail::will_overflow(Packet *const packet) const -> bool {
    if (qib_) {
        return q_->byteLength() + hdr_cmn::access(packet)->size() > qlim_in_bytes();
    } else {
        return q_->length() + 1 > qlim_;
    }
}

auto DropTail::qlim_in_bytes() const -> int {
    return qlim_ * mean_pktsize_;
}

void DropTail::drop_front(Packet *packet) {
    q_->enque(packet);
    Packet *pp = q_->deque();
    drop(pp);
}

void DropTail::drop_prio(Packet *packet) {
    Packet *max_pp = packet;
    int max_prio = 0;

    q_->enque(packet);
    q_->resetIterator();
    for (Packet *pp = q_->getNext(); pp != nullptr; pp = q_->getNext()) {
        if (!qib_ || ( q_->byteLength() - hdr_cmn::access(pp)->size() < qlim_in_bytes())) {
            hdr_ip* h = hdr_ip::access(pp);
            int prio = h->prio();
            if (prio >= max_prio) {
                max_pp = pp;
                max_prio = prio;
            }
        }
    }
    q_->remove(max_pp);
    drop(max_pp);
}

void DropTail::drop_smart(Packet *packet) {
    Packet *max_pp = packet;
    int max_count = 0;

    q_->enque(packet);
    q_->resetIterator();
    for (Packet *pp = q_->getNext(); pp != nullptr; pp = q_->getNext()) {
        hdr_ip* h = hdr_ip::access(pp);
        FlowKey fkey;
        fkey.src = h->saddr();
        fkey.dst = h->daddr();
        fkey.fid = h->flowid();

        char* fkey_buf = (char*) &fkey;
        int length = sizeof(fkey);
        std::string fkey_string(fkey_buf, length);

        std::hash<std::string> string_hasher;
        size_t signature = string_hasher(fkey_string);

        if (sq_counts_.find(signature) != sq_counts_.end()) {
            int count = sq_counts_[signature];
            if (count > max_count) {
                max_count = count;
                max_pp = pp;
            }
        }
    }
    q_->remove(max_pp);
    drop(max_pp);
}

void DropTail::handle_overflow(Packet *packet) {
    if (drop_front_) {
        drop_front(packet);
    }
    else if (drop_prio_) {
        drop_prio(packet);
    }
    else if (drop_smart_) {
        drop_smart(packet);
    } else {
        drop(packet);
    }
}

auto DropTail::deque_priority() -> Packet * {
    auto result = find_best_packet();

    if (!result) {
        return nullptr;
    }

    if (keep_order_) {
        result = find_last_in_flow(result);
    }

    q_->remove(result);
    return result;
}

auto DropTail::find_last_in_flow(Packet *packet) const -> Packet *{
    q_->resetIterator();
    hdr_ip* hp = hdr_ip::access(packet);
    for (Packet *pp = q_->getNext(); pp != packet; pp = q_->getNext()) {
        hdr_ip* h = hdr_ip::access(pp);
        if (is_same_flow(hp, h)) {
            return pp;
        }
    }
    return packet;
}

bool DropTail::is_same_flow(hdr_ip *hp, hdr_ip *h) const {
    return h->saddr() == hp->saddr() && h->daddr() == hp->daddr() && h->flowid() == hp->flowid();
}

DropTail::DropTail() {
    q_ = std::unique_ptr<PacketQueue>(new PacketQueue);
    pq_ = q_.get();
    bind_bool("drop_front_", &drop_front_);
    bind_bool("drop_smart_", &drop_smart_);
    bind_bool("drop_prio_", &drop_prio_);
    bind_bool("deque_prio_", &deque_prio_);
    bind_bool("keep_order_", &keep_order_);
    bind_bool("summarystats_", &summarystats);
    bind_bool("queue_in_bytes_", &qib_);  // boolean: q in bytes?
    bind_bool("ecn_enable_", &ecn_enable_);
    bind("mean_pktsize_", &mean_pktsize_);
    bind("sq_limit_", &sq_limit_);
    bind("thresh_", &thresh_);
}

auto DropTail::find_best_packet() const -> Packet * {
    return find_extreme_packet(ExtremumKind::LOWEST);
}

auto DropTail::get_queue() const -> PacketQueue const * {
    return q_.get();
}

auto DropTail::is_ecn_threshold_reached() const -> bool {
    return q_->length() >= thresh_;
}

void DropTail::mark_ecn() {
    auto const packet = find_extreme_packet(ExtremumKind::HIGHEST);
    if (packet) {
        auto const hf = hdr_flags::access(packet);
        if (hf->ect()) {
            hf->ce() = 1;
        }
    }
}

auto DropTail::find_extreme_packet(ExtremumKind kind) const -> Packet * {
    if (q_->length() == 0) {
        return nullptr;
    }

    q_->resetIterator();
    auto best_packet_ = q_->getNext();
    auto best_prio = hdr_ip::access(best_packet_)->prio();

    for (Packet *pp = q_->getNext(); pp != nullptr; pp = q_->getNext()) {
        auto h = hdr_ip::access(pp);
        auto prio = h->prio();

        //deque from the head
        if ((kind == ExtremumKind ::LOWEST && prio < best_prio) || (kind == ExtremumKind ::HIGHEST && prio > best_prio)) {
            best_packet_ = pp;
            best_prio = prio;
        }
    }
    return best_packet_;
}

auto DropTail::get_queue() -> PacketQueue * {
    return const_cast<PacketQueue *>(const_cast<const DropTail *>(this)->get_queue());
}
