#ifndef DELAY_TRANSPORT_RECEPTIONINFO_H
#define DELAY_TRANSPORT_RECEPTIONINFO_H

namespace delay_transport {

struct ReceptionInfo {
    explicit ReceptionInfo(int hop_count) : hop_count_{hop_count} {}
    auto get_hop_count() const { return hop_count_; }

private:
    int hop_count_;
};

}

#endif //DELAY_TRANSPORT_RECEPTIONINFO_H
