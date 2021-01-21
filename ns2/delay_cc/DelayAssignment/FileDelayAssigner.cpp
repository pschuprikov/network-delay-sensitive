#include "FileDelayAssigner.h"
#include <fstream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <numeric>

namespace {

auto const NUM_BUCKETS = 50;

}

FileDelayAssigner::FileDelayAssigner(std::string const &filename) {
    std::ifstream fin{filename};
    std::map<int, std::vector<double>> num_bytes_to_fcts;

    auto num_flows = 0;

    while (true) {
        double num_packets, flow_duration;
        int rtt_times, dst_from, dst_to;

        fin >> num_packets >> flow_duration >> rtt_times >> dst_from >> dst_to;

        if (!fin) {
            break;
        }

        num_flows++;
        num_bytes_to_fcts[num_packets * 1460].push_back(flow_duration);
    }

    auto const min_flows_per_bucket = num_flows / NUM_BUCKETS;

    std::optional<int> cur_num_bytes{};
    std::vector<double> current_bucket{};
    for (auto& [num_bytes, fcts] : num_bytes_to_fcts) {
        cur_num_bytes = num_bytes;
        std::copy(begin(fcts), end(fcts), std::back_inserter(current_bucket));
        if (int(current_bucket.size()) >= min_flows_per_bucket) {
            deadlines_[*cur_num_bytes] = *std::max_element(begin(current_bucket), end(current_bucket));
            cur_num_bytes = std::nullopt;
            current_bucket.clear();
        }
    }
    if (!current_bucket.empty()) {
        deadlines_[*cur_num_bytes] = *std::max_element(begin(current_bucket), end(current_bucket));
    }
}

double FileDelayAssigner::assign_delay(int msg_len) {
    auto len_it = deadlines_.lower_bound(msg_len);
    if (len_it == end(deadlines_)) {
        len_it--;
    }
    return len_it->second;
}
