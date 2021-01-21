#ifndef NS2_DROP_TAIL_D_H
#define NS2_DROP_TAIL_D_H

#include <delay_cc/queue/mixins/ExpirationTrackingMixin.h>
#include <delay_cc/queue/mixins/DropInfoMixin.h>
#include <delay_cc/queue/utils/ExpirationTimeController.h>
#include <delay_cc/queue/utils/LastDelayController.h>
#include <delay_transport/DropInfo.h>
#include <queue/drop-tail.h>
#include <memory>

class DDropTail : public DropInfoMixin<ExpirationTrackingMixin<DropTail>> {
    using super = DropInfoMixin;

    enum class DelayEstimationMode : int {
        LOCAL, GLOBAL, MINIMAL
    };

public:
    DDropTail();

    void enque(Packet * packet) override;
    auto deque() -> Packet * override;

protected:
    auto will_overflow(Packet * packet) const -> bool override;

    void handle_overflow(Packet * packet) override;

private:
    /**
     * Performs additional admission check for a delay packet
     *
     * For example, if it is going to be expired if followed FIFO, e.t.c
     *
     * \returns a drop reason if a drop decision has been taken
     */
    auto check_delay_packet_admission(Packet * packet) const
        -> optional<delay_transport::DropReason>;

    /**
     * Delegates admission check to an expiration time controller
     */
    auto check_admission_by_expiration_time_controller(Packet * packet) const
        -> optional<delay_transport::DropReason>;

    /**
     * Updates control state on delay packet admission
     */
    void pre_enque_delay_packet(Packet * packet);

    /**
     * Estimates packet delay at the switch
     *
     * That is queueing + serialization (assuming store and forward)
     */
    auto calc_local_expected_delay(Packet * packet) const -> double;

    /**
     * Estimates packet delay at this _and_ following switches
     *
     * That is, queueing + serialization (assuming store and forward)
     */
    auto calc_global_expected_delay(Packet * packet) const -> double;

    auto calc_minimal_delay(Packet * packet) const -> double;

    auto get_expected_delay(Packet *packet) const -> double;

    /**
     * Removes expired packets from the queue
     * \returns true if at least one packet was removed
     */
    auto remove_expired_from_queue(delay_transport::DropReason reason) -> bool;

private:
    DelayEstimationMode delay_estimation_mode_;
    int last_delay_controller_enabled_;
    int expiration_time_controller_enabled_;

    LastDelayController last_delay_controller_;
    std::unique_ptr<ExpirationTimeController> expiration_time_controller_;
};

#endif //NS2_DROP_TAIL_D_H
