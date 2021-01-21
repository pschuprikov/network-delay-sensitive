#ifndef NS2_TCP_HEADER_H
#define NS2_TCP_HEADER_H

#include "packet.h"

/* these are used to mark packets as to why we xmitted them */
enum class TcpXmissionReason : uint8_t {
    NORMAL     = 0x00,
    TIMEOUT    = 0x01,
    DUPACK     = 0x02,
    RBP        = 0x03,
    PARTIALACK = 0x04,
    SACK       = 0x05
};

struct hdr_tcp {
#define NSA 3
    double ts_;             /* time packet generated (at source) */
    double ts_echo_;        /* the echoed timestamp (originally sent by
	                           the peer) */
    int seqno_;             /* sequence number */
    TcpXmissionReason reason_;            /* reason for a retransmit */
    int sack_area_[NSA+1][2];	/* sack blocks: start, end of block */
    int sa_length_;         /* Indicate the number of SACKs in this  *
	                         * packet.  Adds 2+sack_length*8 bytes   */
    int ackno_;             /* ACK number for FullTcp */
    int hlen_;              /* header len (bytes) for FullTcp */
    int tcp_flags_;         /* TCP flags for FullTcp */
    int last_rtt_;		/* more recent RTT measurement in ms, */
    /*   for statistics only */

    static int offset_;	// offset for this header
    inline static int& offset() { return offset_; }
    inline static hdr_tcp* access(Packet* p) {
        return (hdr_tcp*) p->access(offset_);
    }

    /* per-field member functions */
    double& ts() { return (ts_); }
    double& ts_echo() { return (ts_echo_); }
    int& seqno() { return (seqno_); }
    TcpXmissionReason& reason() { return (reason_); }
    int& sa_left(int n) { return (sack_area_[n][0]); }
    int& sa_right(int n) { return (sack_area_[n][1]); }
    int& sa_length() { return (sa_length_); }
    int& hlen() { return (hlen_); }
    int& ackno() { return (ackno_); }
    int& flags() { return (tcp_flags_); }
    int& last_rtt() { return (last_rtt_); }
};

#endif //NS2_TCP_HEADER_H
