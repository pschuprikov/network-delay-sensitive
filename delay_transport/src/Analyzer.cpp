#include "Analyzer.h"
#include "AnalysisMetadataImpl.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <delay_transport/DropInfo.h>
#include <iostream>
#include <numeric>

namespace delay_transport {

delay_transport::Analyzer::Analyzer(Environment *environment, double start_time,
                                    double report_delay)
      : environment{environment},
        report_delay{report_delay},
        timer{environment->create_timer()},
        num_retransmission_drops{0},
        num_new_messages{},
        num_deadline_satisfied{0},
        sender_deliveries{0},
        receiver_deliveries{0},
        packets_sent{},
        drop_count{},
        priority_used{},
        in_time_packets{},
        out_of_time_packets{},
        before_time_packets{},
        drop_by_last_byte{},
        drop_by_slack_fraction{},
        expiration_remaining_slack_fraction{},
        expiration_remaining_data_fraction{},
        first_delivery_remaining_slack_fraction{} {
    timer->reschedule(
            std::max(0.0, start_time - environment->get_current_time()),
            [this] {
                cleanup();
                reschedule_report();
            });
}

void delay_transport::Analyzer::on_drop(
        delay_transport::DelayPacketMetadata const &packet,
        delay_transport::DropInfo const &info) {
    auto const analysis_meta = dynamic_cast<AnalysisMetadataImpl *>(
            packet.analysis_metadata.get());
    violated_slacks[get_group(packet.message_len)].push_back(info.last_slack);
    if (info.expiration_time > 0.0) {
        expiration_times.push_back(info.expiration_time);
    }
    int const slack_bucket = std::max(
            0, std::min<int>(
                       NUM_BUCKETS - 1,
                       floor(NUM_BUCKETS *
                             (packet.expiration_time -
                              analysis_meta->message_creation_time) /
                             (double)(analysis_meta->message_expiration_time -
                                      analysis_meta->message_creation_time))));
    drop_by_slack_fraction[get_group(packet.message_len)][slack_bucket]++;
    message_sizes.push_back(packet.message_len);
    hop_idx[info.hop_idx]++;
    drop_count[get_group(packet.message_len)]++;
    retransmission_indexes[get_group(packet.message_len)].push_back(
            packet.retransmission_idx);
    if (packet.is_retransmission()) {
        num_retransmission_drops++;
    }
}

void delay_transport::Analyzer::report() {
    auto const total_drop_count =
            std::accumulate(begin(drop_count), end(drop_count), 0);
    auto const avg_expiration_time =
            std::accumulate(begin(expiration_times), end(expiration_times),
                            0.0) /
            size(expiration_times);

    std::cerr << "STATS"
              << " ( " << environment->get_current_time() << " )\n";
    for (int group = 0; group < NUM_GROUPS; group++) {
        std::sort(begin(retransmission_indexes[group]),
                  end(retransmission_indexes[group]));
        std::sort(begin(violated_slacks[group]), end(violated_slacks[group]));

        std::cerr << "== group " << (group + 1) << " ==\n";
        std::cerr << "new messages: " << num_new_messages[group] << "\n";
        std::cerr << "successful (snd) deliveries: " << sender_deliveries[group]
                  << " (in time: " << num_deadline_satisfied[group] << ")\n";
        std::cerr << "successful (rcv) deliveries: "
                  << receiver_deliveries[group] << "\n";
        std::cerr << "packets sent: " << packets_sent[group] << "\n";
        std::cerr << "drop count: " << drop_count[group] << "\n";
        if (!retransmission_indexes[group].empty()) {
            std::cerr
                    << "median retransmission idx: "
                    << retransmission_indexes
                               [group][retransmission_indexes[group].size() / 2]
                    << "\n";
        }
        std::cerr << "first delivery slack: [ ";
        for (auto x : first_delivery_remaining_slack_fraction[group]) {
            std::cerr << x << " ";
        }
        std::cerr << "]\n";
        std::cerr << "expiration distr: (" << before_time_packets[group]
                  << ") [ ";
        for (auto x : in_time_packets[group]) {
            std::cerr << x << " ";
        }
        std::cerr << "] (" << out_of_time_packets[group] << ")\n";

        std::cerr << "expiration remaining slack fraction: ";
        for (auto x : expiration_remaining_slack_fraction[group]) {
            std::cerr << x << " ";
        }
        std::cerr << "\n";

        std::cerr << "expiration remaining data fraction: ";
        for (auto x : expiration_remaining_data_fraction[group]) {
            std::cerr << x << " ";
        }
        std::cerr << "\n";

        std::cerr << "drop/last_byte distr: [ ";
        for (auto x : drop_by_last_byte[group]) {
            std::cerr << x << " ";
        }
        std::cerr << "]\n";

        std::cerr << "drop/slack fraction distr: [ ";
        for (auto x : drop_by_slack_fraction[group]) {
            std::cerr << x << " ";
        }
        std::cerr << "]\n";

        std::cerr << "drop median slack: ";
        if (!violated_slacks[group].empty()) {
            auto const median_slack =
                    violated_slacks[group][violated_slacks[group].size() / 2];
            std::cerr << median_slack;
        } else {
            std::cerr << "NA";
        }
        std::cerr << "\n";

        std::cerr << "avg fraction in time: ";
        if (!fraction_in_time[group].empty()) {
            std::cerr << std::accumulate(begin(fraction_in_time[group]),
                                         end(fraction_in_time[group]), 0.0) /
                                 fraction_in_time[group].size();
        } else {
            std::cerr << "NA";
        }
        std::cerr << "\n";

        std::cerr << "avg priority: ";
        if (!priority_used[group].empty()) {
            std::cerr << std::accumulate(begin(priority_used[group]),
                                         end(priority_used[group]), 0) /
                                 (double)priority_used[group].size();
        } else {
            std::cerr << "NA";
        }
        std::cerr << "\n";
    }

    std::cerr << "== overall ==\n";
    std::cerr << "packets sent: "
              << std::accumulate(begin(packets_sent), end(packets_sent), 0)
              << "\n";
    std::cerr << "avg expiration time " << avg_expiration_time << "\n";
    if (!message_sizes.empty()) {
        std::sort(begin(message_sizes), end(message_sizes));
        auto const median_message_size =
                message_sizes[message_sizes.size() / 2];
        std::cerr << "drop median msg size: " << median_message_size << "\n";
    }

    std::cerr << "drop count: " << total_drop_count
              << " (retransmissions: " << num_retransmission_drops << ")\n";
    for (auto [idx, cnt] : hop_idx) {
        std::cerr << "hop idx = " << idx << "; count: " << cnt << "\n";
    }
    std::cerr << "===========\n";

    cleanup();

    reschedule_report();
}

void Analyzer::cleanup() {
    std::fill(begin(sender_deliveries), end(sender_deliveries), 0);
    std::fill(begin(receiver_deliveries), end(receiver_deliveries), 0);
    std::fill(begin(num_deadline_satisfied), end(num_deadline_satisfied), 0);
    std::fill(begin(packets_sent), end(packets_sent), 0);
    std::fill(begin(drop_count), end(drop_count), 0);
    std::fill(begin(retransmission_indexes), end(retransmission_indexes),
              std::vector(0, 0));
    std::fill(begin(fraction_in_time), end(fraction_in_time),
              std::vector(0, 0.0));
    std::fill(begin(num_new_messages), end(num_new_messages), 0);
    for (auto &bucket : in_time_packets) {
        std::fill(begin(bucket), end(bucket), 0);
    }
    for (auto &bucket : drop_by_last_byte) {
        std::fill(begin(bucket), end(bucket), 0);
    }
    for (auto &bucket : drop_by_slack_fraction) {
        std::fill(begin(bucket), end(bucket), 0);
    }
    for (auto &bucket : expiration_remaining_slack_fraction) {
        std::fill(begin(bucket), end(bucket), 0);
    }
    for (auto &bucket : expiration_remaining_data_fraction) {
        std::fill(begin(bucket), end(bucket), 0);
    }
    for (auto &bucket : first_delivery_remaining_slack_fraction) {
        std::fill(begin(bucket), end(bucket), 0);
    }

    std::fill(begin(out_of_time_packets), end(out_of_time_packets), 0);
    std::fill(begin(before_time_packets), end(before_time_packets), 0);
    std::fill(begin(violated_slacks), end(violated_slacks),
              std::vector<double>{});
    std::fill(begin(priority_used), end(priority_used), std::vector<int>{});

    num_retransmission_drops = 0;
    expiration_times.clear();
    message_sizes.clear();
    hop_idx.clear();
}

void Analyzer::reschedule_report() {
    timer->reschedule(report_delay, [this] { report(); });
}

void Analyzer::on_sender_delivery(
        delay_transport::ApplicationMessageData const &app) {
    if (environment->get_current_time() <= app.get_expiration_time()) {
        num_deadline_satisfied[get_group(app.length_in_bytes)]++;
    }
    sender_deliveries[get_group(app.length_in_bytes)]++;
}

void Analyzer::on_packet_sent(
        delay_transport::DelayPacketMetadata const &packet) {
    auto const analysis_meta = dynamic_cast<AnalysisMetadataImpl *>(
            packet.analysis_metadata.get());
    if (packet.expiration_time > analysis_meta->message_expiration_time) {
        out_of_time_packets[get_group(packet.message_len)]++;
    } else if (packet.expiration_time < environment->get_current_time()) {
        before_time_packets[get_group(packet.message_len)]++;
    } else {
        int const bucket = floor(NUM_BUCKETS *
                                 (packet.expiration_time -
                                  analysis_meta->message_creation_time) /
                                 (analysis_meta->message_expiration_time -
                                  analysis_meta->message_creation_time));
        if (packet.expiration_time == std::numeric_limits<double>::infinity()) {
            in_time_packets[get_group(packet.message_len)].back()++;
        } else {
            in_time_packets[get_group(packet.message_len)][bucket]++;
        }
    }
    priority_used[get_group(packet.message_len)].push_back(
            static_cast<int>(packet.get_priority()));
    packets_sent[get_group(packet.message_len)]++;
}

void Analyzer::on_new_message(
        delay_transport::ApplicationMessageData const &app) {
    num_new_messages[get_group(app.length_in_bytes)]++;
}

int Analyzer::get_group(int msg_size) {
    if (msg_size <= 100 * 1024) {
        return 0;
    }
    if (msg_size <= 10 * 1024 * 1024) {
        return 1;
    }

    return 2;
}

void Analyzer::on_receiver_delivery(int message_len) {
    receiver_deliveries[get_group(message_len)]++;
}

void Analyzer::on_message_timeout(int message_len, int bytes_delivered) {
    fraction_in_time[get_group(message_len)].push_back(bytes_delivered /
                                                       (double)message_len);
}

Analyzer::~Analyzer() { report(); }

void Analyzer::report_expiration(ApplicationMessageData const &app,
                                 int bytes_remaining) {
    // int const slack_bucket = floor(slack_fraction * NUM_BUCKETS);
    // expiration_remaining_slack_fraction[get_group(app.length_in_bytes)][slack_bucket]++;
    auto const data_fraction = bytes_remaining / (double)app.length_in_bytes;
    int const data_bucket =
            std::min<int>(floor(data_fraction * NUM_BUCKETS), NUM_BUCKETS - 1);
    expiration_remaining_data_fraction[get_group(app.length_in_bytes)]
                                      [data_bucket]++;
}

void Analyzer::on_first_delivery(
        delay_transport::DelayPacketMetadata const &packet) {
    auto const analysis_meta = dynamic_cast<AnalysisMetadataImpl *>(
            packet.analysis_metadata.get());
    auto const slack_fraction = (environment->get_current_time() -
                                 analysis_meta->message_creation_time) /
                                (analysis_meta->message_expiration_time -
                                 analysis_meta->message_creation_time);
    auto const slack_bucket =
            std::min(int(floor(slack_fraction * NUM_BUCKETS)), NUM_BUCKETS);
    first_delivery_remaining_slack_fraction[get_group(packet.message_len)]
                                           [slack_bucket]++;
}

} // namespace delay_transport
