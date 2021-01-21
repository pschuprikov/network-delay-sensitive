/*
 * Strict Priority Queueing (SP)
 *
 * Variables:
 * queue_num_: number of Class of Service (CoS) queues
 * thresh_: ECN marking threshold
 * mean_pktsize_: configured mean packet size in bytes
 * marking_scheme_: Disable ECN (0), Per-queue ECN (1) and Per-port ECN (2)
 */

#ifndef ns_priority_h
#define ns_priority_h

#include "queue.h"
#include "config.h"
#include <memory>

class Priority : public Queue {
    public:
        static auto const MAX_QUEUE_NUM = 8;

        enum class ECNMode : int {
            DISABLE = 0,
            PER_QUEUE = 1,
            PER_PORT = 2
        };

    public:
        Priority();

        void enque(Packet * packet) override;
        auto deque() -> Packet * override;

    protected:
        virtual void handle_overflow(Packet * packet);
        virtual int get_priority(Packet * packet) const;

        auto clamp_priority(int priority) const -> int;

    protected:
        std::array<std::unique_ptr<PacketQueue>, MAX_QUEUE_NUM> q_;     // underlying multi-FIFO (CoS) queues
        int mean_pktsize_;      // configured mean packet size in bytes
        int thresh_;            // single ECN marking threshold
        int queue_num_;         // number of CoS queues. No more than MAX_QUEUE_NUM
        ECNMode marking_scheme_;    // Disable ECN (0), Per-queue ECN (1) and Per-port ECN (2)


        //Return total queue length (bytes) of all the queues
        int TotalByteLength();

    private:

        void ensure_queue_bound();
};

#endif
