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
 *
 * @(#) $Header: /cvsroot/nsnam/ns-2/queue/drop-tail.h,v 1.19 2004/10/28 23:35:37 haldar Exp $ (LBL)
 */

#ifndef ns_drop_tail_h
#define ns_drop_tail_h

#include <unordered_map>
#include <functional>
#include <queue>

using std::queue;

#include <memory>
#include <string>
#include "queue.h"
#include "config.h"

typedef struct flowkey {
	nsaddr_t src, dst;
	int fid;
} FlowKey;

/*
 * A bounded, drop-tail queue
 */
class DropTail : public Queue {
public:
    enum class ExtremumKind {
        HIGHEST, LOWEST
    };

public:
    DropTail();

	void reset() override;

    auto qlim_in_bytes() const -> int;

	int command(int argc, const char*const* argv) override;

	void enque(Packet*) override;

	Packet* deque() override;


protected:
	void shrink_queue();

    /** 
     * Must always be called on enque.
     */
    void pre_enque();

	auto get_queue() const -> PacketQueue const *;
    auto get_queue() -> PacketQueue *;
	int get_mean_packet_size() const { return mean_pktsize_; }
	bool get_queue_measured_in_bytes() const { return qib_; }

    virtual auto will_overflow(Packet * packet) const -> bool;
    virtual void handle_overflow(Packet * packet);

private:
    auto find_best_packet() const -> Packet *;
    auto find_extreme_packet(ExtremumKind kind) const -> Packet *;
    auto find_last_in_flow(Packet *packet) const -> Packet *;

    auto is_ecn_threshold_reached() const -> bool;
    void mark_ecn();

    void drop_front(Packet * packet);
    void drop_prio(Packet * packet);
    void drop_smart(Packet * packet);

    auto deque_priority() -> Packet *;
    auto deque_with_drop_smart() -> Packet *;

    void print_summary_stats();

    bool is_same_flow(hdr_ip *hp, hdr_ip *h) const;

private:
	std::unique_ptr<PacketQueue> q_;	/* underlying FIFO queue */

    int thresh_;
	int drop_front_;	/* drop-from-front (rather than from tail) */
	int summarystats;
	int qib_;       	/* bool: queue measured in bytes? */
	int mean_pktsize_;	/* configured mean packet size in bytes */
	// Mohammad: for smart dropping
	int drop_smart_;
	// Shuang: for priority dropping
	int drop_prio_;
	int deque_prio_;
	int keep_order_;
	int ecn_enable_;

	unsigned int sq_limit_;
    std::unordered_map<size_t, int> sq_counts_;
	std::queue<size_t> sq_queue_;
};

#endif
