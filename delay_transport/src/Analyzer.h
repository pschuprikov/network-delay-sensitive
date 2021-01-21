#ifndef DELAY_TRANSPORT_ANALYZER_H
#define DELAY_TRANSPORT_ANALYZER_H

#include <delay_transport/ApplicationMessageData.h>
#include <delay_transport/DelayPacketMetadata.h>
#include <delay_transport/DropInfo.h>
#include <delay_transport/Environment.h>
#include <delay_transport/Timer.h>

#include "ExpirationListener.h"
#include <array>
#include <map>
#include <memory>
#include <set>

namespace delay_transport {

class Analyzer : public ExpirationListener {
  public:
    Analyzer(Environment *environment, double start_time, double report_delay);
    ~Analyzer() override;

    void on_new_message(delay_transport::ApplicationMessageData const &app);
    void on_drop(delay_transport::DelayPacketMetadata const &packet,
                 delay_transport::DropInfo const &info);
    void on_sender_delivery(delay_transport::ApplicationMessageData const &app);
    void on_receiver_delivery(int message_len);
    void on_packet_sent(delay_transport::DelayPacketMetadata const &packet);
    void on_message_timeout(int message_len, int bytes_delivered);
    void on_first_delivery(delay_transport::DelayPacketMetadata const &packet);

    void report_expiration(ApplicationMessageData const &app,
                           int bytes_remaining) override;

  private:
    static auto const constexpr NUM_GROUPS = 3;
    static auto const constexpr NUM_BUCKETS = 10;

    void report();
    void cleanup();
    void reschedule_report();

    static int get_group(int msg_size);

  private:
    Environment const *environment;

    double const report_delay;
    std::shared_ptr<Timer> const timer;

    std::array<std::vector<double>, NUM_GROUPS> violated_slacks;
    std::array<std::vector<double>, NUM_GROUPS> fraction_in_time;
    std::vector<double> expiration_times;
    std::vector<int> message_sizes;

    std::map<int, int> bytes_sent;

    std::map<int, int> hop_idx;
    int num_retransmission_drops;
    std::array<int, NUM_GROUPS> num_new_messages;
    std::array<int, NUM_GROUPS> num_deadline_satisfied;
    std::array<int, NUM_GROUPS> sender_deliveries;
    std::array<int, NUM_GROUPS> receiver_deliveries;
    std::array<int, NUM_GROUPS> packets_sent;
    std::array<int, NUM_GROUPS> drop_count;
    std::array<std::vector<int>, NUM_GROUPS> priority_used;
    std::array<std::vector<int>, NUM_GROUPS> retransmission_indexes;
    std::array<std::array<int, NUM_BUCKETS>, NUM_GROUPS> in_time_packets;
    std::array<int, NUM_GROUPS> out_of_time_packets;
    std::array<int, NUM_GROUPS> before_time_packets;
    std::array<std::array<int, NUM_BUCKETS>, NUM_GROUPS> drop_by_last_byte;
    std::array<std::array<int, NUM_BUCKETS>, NUM_GROUPS> drop_by_slack_fraction;
    std::array<std::array<int, NUM_BUCKETS>, NUM_GROUPS>
            expiration_remaining_slack_fraction;
    std::array<std::array<int, NUM_BUCKETS>, NUM_GROUPS>
            expiration_remaining_data_fraction;
    std::array<std::array<int, NUM_BUCKETS>, NUM_GROUPS>
            first_delivery_remaining_slack_fraction;
};

} // namespace delay_transport

#endif // DELAY_TRANSPORT_ANALYZER_H
