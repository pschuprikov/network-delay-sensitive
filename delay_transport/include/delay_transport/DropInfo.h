#ifndef DELAY_TRANSPORT_DROPINFO_H
#define DELAY_TRANSPORT_DROPINFO_H

namespace delay_transport {
enum class DropReason {
    OVERFLOW, EXPIRED_ENQUE, EXPIRED_DEQUE, ADMISSION_POLICY
};

struct DropInfo {
   DropReason reason;
   int hop_idx;
   double last_slack;
   double expiration_time;
};

}

#endif //DELAY_TRANSPORT_DROPINFO_H
