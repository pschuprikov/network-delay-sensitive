#include "DFullTcpAgent.h"

#include <delay_cc/tcp/mixins/BindCachingMixin.hpp>

#include <fstream>

template<class Base>
DFullTcpAgent<Base>::DFullTcpAgent() 
    : super{}, peer_{nullptr} {
}

template<class Base>
void DFullTcpAgent<Base>::advance_bytes(int num_bytes) {
    this->nominal_deadline = calc_nominal_deadline(num_bytes);
    this->deadline = this->nominal_deadline;

    if (delay_transport_) {
        delay_transport_->initialize(num_bytes, this->nominal_deadline);
    }

    super::advance_bytes(num_bytes);
}

template<class Base>
auto DFullTcpAgent<Base>::get_peer() -> DFullTcpAgent * {
    if (!peer_) {
        auto& tcl = Tcl::instance();
        tcl.evalf("[Simulator instance] get-node-by-addr %d", this->daddr());
        tcl.evalf("%s agent %d", tcl.result(), this->dport()); 
        peer_ = dynamic_cast<DFullTcpAgent *>(tcl.lookup(tcl.result()));

        if (peer_ == nullptr) {
            throw new std::runtime_error("couldn't find peer");
        }
    }
    return peer_;
}

template<class Base>
auto DFullTcpAgent<Base>::get_num_buffered_bytes() -> int {
    return this->highest_ack_ < 0 ? this->curseq_ - this->iss_ : this->curseq_ - this->highest_ack_ + 1;
}

template<class Base>
auto DFullTcpAgent<Base>::get_max_bytes_in_packet() const -> int {
    return this->maxseg_;
}

template<class Base>
auto DFullTcpAgent<Base>::link_speed_in_gbps() const -> int {
    return this->link_rate_;
}

template<class Base>
auto DFullTcpAgent<Base>::calc_nominal_deadline(int num_bytes) -> int {
    if (delay_assigner_) {
        auto deadline_in_sec = delay_assigner_->assign_delay(num_bytes);
        if (deadline_in_sec == std::numeric_limits<decltype(deadline_in_sec)>::infinity()) {
            return 0;
        } else {
            return static_cast<int>(deadline_in_sec * 1e6);
        }
    }
    return 0;
}

template<class Base>
auto DFullTcpAgent<Base>::command(int argc, const char*const* argv) -> int {
    if (argc == 2) {
        if (strcmp(argv[1], "enable-delay") == 0) {
            assert(!delay_transport_);
            delay_transport_ = std::make_shared<DelayTransport>(this);
            return TCL_OK;
        }
    }
    if (argc == 3) {
        if (strcmp(argv[1], "set-delay-assigner") == 0) {
            nlohmann::json json;
            std::ifstream(argv[2]) >> json;
            delay_assigner_ = DelayAssigner::create_from_json(json, this);
            return TCL_OK;
        }
    }
    if (strcmp(argv[1], "delay-command") == 0) {
        if (delay_transport_) {
            return delay_transport_->command(argc - 1, argv + 1);
        }
        return TCL_OK;
    }
    return super::command(argc, argv);
}

template<class Base>
void DFullTcpAgent<Base>::recv(Packet * pkt, Handler * h) {
    if (delay_transport_) {
        delay_transport_->on_recv(pkt);
    }

    super::recv(pkt, h);
}

template<class Base>
void DFullTcpAgent<Base>::delay_bind_init_all() {
    this->delay_bind_init_one("enable_early_expiration_");
    super::delay_bind_init_all();
}

template<class Base>
auto DFullTcpAgent<Base>::get_oracle_fct(int num_bytes) -> double {
    auto const maxseg = this->get_max_bytes_in_packet();
    auto const link_rate = this->link_speed_in_gbps();
    auto const header_size = this->headersize();
    auto const num_packets = num_bytes / maxseg;
    auto const left_over = num_bytes - maxseg * num_packets;
    auto const incl_overhead_bytes =
            num_packets * (maxseg + header_size) + (left_over + header_size);
    auto transmission_delay =
            (incl_overhead_bytes + 2.0 * header_size) * 8.0  * 1.e-9 / link_rate;
    if (num_packets == 0) {
        transmission_delay += 2 * (left_over + 2 * header_size) * 8.0 * 1.e-9 / (4 * link_rate);
    } else {
        transmission_delay += 2 * (maxseg + 2 * header_size) * 8.0 * 1.e-9 / (4 * link_rate);
    }

    auto const propagation_delay = 4 * 0.00002 + 8 * 0.0000002;

    return propagation_delay + transmission_delay;
}

template<class Base>
auto DFullTcpAgent<Base>::delay_bind_dispatch(
        const char * varName, 
        const char * localName,
        TclObject * tracer
        ) -> int {
    if (this->delay_bind_bool(varName, localName, "enable_early_expiration_", &enable_early_expiration_, tracer)) {
        return TCL_OK;
    }
    return super::delay_bind_dispatch(varName, localName, tracer);
}


template<class Base>
void DFullTcpAgent<Base>::bufferempty() {
    if (delay_transport_) {
        delay_transport_->on_buffer_empty();
    }
    super::bufferempty();
}


template<class Base>
void DFullTcpAgent<Base>::sendpacket(
        int seqno, int ackno, int pflags, int datalen, 
        TcpXmissionReason reason, Packet *p) {
    if (!p) {
        p = super::allocpkt();
    }

    if (delay_transport_ && datalen > 0 && !is_stale_data(seqno)) {
        delay_transport_->on_packet_send(p,
                seqno - this->startseq_,
                seqno - this->startseq_ + datalen - 1);
    }

    super::sendpacket(seqno, ackno, pflags, datalen, reason, p);
}

template<class Base>
auto DFullTcpAgent<Base>::is_stale_data(int seqno) const -> bool { 
    return seqno <= this->startseq_; 
}

template<class Base>
auto DFullTcpAgent<Base>::foutput(int seqno, TcpXmissionReason reason) -> int {
    assert(enable_early_expiration_ == 0);
    if (enable_early_expiration_ == 0 || this->deadline == 0) {
        return super::foutput(seqno, reason);
    }

    auto const cur_time_us = int(Scheduler::instance().clock() * 1e6);
    if (cur_time_us >= this->get_expiration_time_us()) {
        if (this->signal_on_empty_ && this->iss_ != seqno) {
            reset_source();
            get_peer()->reset_target(this->curseq_ + 1);

            this->early_terminated_ = true;;
            this->bufferempty();
            this->early_terminated_ = false;

            return 0;
        }
    }

    return super::foutput(seqno, reason);
}

template<class Base>
void DFullTcpAgent<Base>::reset_source() {
    auto const next_seq = this->curseq_ + 1; 
    this->pipe_ = 0;
    this->sack_min_ = next_seq;
    this->highest_ack_ = next_seq;
    this->t_seqno_ = next_seq;
    this->sq_.clear();
    this->h_seqno_ = next_seq;
}

template<class Base>
void DFullTcpAgent<Base>::reset_target(int next_seq) {
    if constexpr(std::is_base_of_v<SackFullTcpAgent, Base>) {
        this->last_ack_sent_ = next_seq;
        this->rcv_nxt_ = next_seq;
    }
}
