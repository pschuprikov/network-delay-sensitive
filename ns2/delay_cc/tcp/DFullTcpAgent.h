#ifndef NS2_DFULLTCPAGENT_H
#define NS2_DFULLTCPAGENT_H

#include <delay_cc/DelayAssignment/FileDelayAssigner.h>
#include <delay_cc/DelayTransport.h>

#include <tcp/tcp-full.h>
#include <tcp/tcp_header.h>

template<class Base>
class DFullTcpAgent : public Base, public TopologyOracle, public AgentInfoProvider {
    using super = Base;
    static_assert(std::is_base_of_v<FullTcpAgent, Base>);
public:
    DFullTcpAgent();

    void advance_bytes(int num_bytes) override; 

    int command(int argc, const char*const* argv) override;

    void recv(Packet * pkt, Handler * h) override;

    auto get_oracle_fct(int num_bytes) -> double override;

    [[nodiscard]]
    auto get_max_bytes_in_packet() const -> int override;

    auto get_num_buffered_bytes() -> int override;

    [[nodiscard]]
    auto link_speed_in_gbps() const -> int override;

protected:
    void delay_bind_init_all() override; 

    auto delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer) -> int override;

    void bufferempty() override; 

    void sendpacket(int seqno, int ackno, int pflags, int datalen, TcpXmissionReason reason, Packet *p) override;

private:
    void reset_source();
    void reset_target(int source_curseq);

    int foutput(int seqno, TcpXmissionReason reason) override;

    auto is_stale_data(int seqno) const -> bool;

    auto calc_nominal_deadline(int num_bytes) -> int;

    auto get_peer() -> DFullTcpAgent *;

private:
    int enable_early_expiration_;
    DFullTcpAgent * peer_;
    std::shared_ptr<DelayAssigner> delay_assigner_;
    std::shared_ptr<DelayTransport> delay_transport_;
};


#endif //NS2_DFULLTCPAGENT_H
