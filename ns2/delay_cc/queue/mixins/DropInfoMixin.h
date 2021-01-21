#ifndef NS2_DROPINFOMIXIN_H
#define NS2_DROPINFOMIXIN_H

#include <delay_transport/DropInfo.h>
#include <delay_cc/hdr_delay.h>
#include <delay_cc/DelayTransport.h>
#include "queue.h"
#include "packet.h"


template<class Base>
class DropInfoMixin : public Base {
public:
    virtual void drop_with_reason(
            Packet * packet, 
            delay_transport::DropReason drop_reason); 

    void drop(Packet * packet) final {
        drop_with_reason(packet, delay_transport::DropReason::OVERFLOW);
    }

protected:

    virtual void on_drop([[maybe_unused]] Packet * packet) { }

private:
    void queue_drop_delay_packet(
            Packet *packet, 
            delay_transport::DropReason drop_reason); 
};

#endif //NS2_DROPINFOMIXIN_H
