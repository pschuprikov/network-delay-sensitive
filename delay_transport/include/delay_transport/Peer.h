#ifndef DELAY_TRANSPORT_PEER_H
#define DELAY_TRANSPORT_PEER_H

namespace delay_transport {

struct Peer {
    virtual bool equal(Peer const &other) const = 0;

    Peer() = default;

    Peer(Peer const &) = default;

    Peer const &operator=(Peer const &) = delete;

    virtual ~Peer() = default;
};

}

#endif //DELAY_TRANSPORT_PEER_H
