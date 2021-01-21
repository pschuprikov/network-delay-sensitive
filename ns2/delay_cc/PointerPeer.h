#ifndef NS2_POINTERPEER_H
#define NS2_POINTERPEER_H

#include <delay_transport/Peer.h>
#include "DelayTransport.h"

class PointerPeer : public delay_transport::Peer {
public:
    explicit PointerPeer(DelayTransport *transport = nullptr);

    [[nodiscard]] 
    auto equal(Peer const &other) const -> bool override;

    [[nodiscard]] 
    auto get_transport() const -> DelayTransport *;

private:
    DelayTransport *transport;
};


#endif //NS2_POINTERPEER_H
