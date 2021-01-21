#ifndef NS2_HDR_DELAY_H
#define NS2_HDR_DELAY_H

#include <delay_transport/DelayPacketMetadata.h>
#include <common/packet.h>
#include <delay_cc/DelayTransport_fwd.h>

class hdr_delay {
public:
    inline static auto access(Packet * p) -> hdr_delay * {
        return reinterpret_cast<hdr_delay *>(p->access(offset_));
    }

    auto transport() -> DelayTransport * & { return transport_; }
    auto expiration_time() -> delay_transport::ExpirationTime & { return expiration_time_; }
    delay_transport::ExpirationTime expiration_time() const { return expiration_time_; }

    std::shared_ptr<delay_transport::AnalysisMetadata>& analysis_meta() { return meta_; }
    int& message_id() { return message_id_; }
    int& message_length() { return message_length_; }
    int& first_byte() { return first_byte_; }
    int& last_byte() { return last_byte_; }
    int hop_count() { return hop_count_; }
    int& retransmission_idx() { return retransmission_idx_; }
    auto priority() const { return priorities_[hop_count_]; }
    auto& priorities() { return priorities_; }
    double last_slack() const { return last_slack_; }

    void increment_hop_count();

    void update_last_slack();
    double current_slack() const;
    bool is_expired() const;
    bool will_expire(double expected_delay) const;
    bool is_present() const { return transport_; }
    double& creation_time() { return creation_time_; }

private:
    inline static int const& offset() { return offset_; };
    friend class DelayHeaderClass;

private:
    static int offset_;

private:
    double last_slack_;
    delay_transport::ExpirationTime expiration_time_;
    std::shared_ptr<delay_transport::AnalysisMetadata> meta_;
    std::array<delay_transport::Priority, 4> priorities_;
    double creation_time_;
    int message_id_;
    int message_length_;

    int first_byte_;
    int last_byte_;

    int retransmission_idx_;

    DelayTransport * transport_;
    int hop_count_;
};


#endif //NS2_HDR_DELAY_H
