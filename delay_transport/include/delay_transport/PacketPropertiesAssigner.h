#ifndef DELAY_TRANSPORT_PACKETPROPERTIESASSIGNER_H
#define DELAY_TRANSPORT_PACKETPROPERTIESASSIGNER_H

namespace delay_transport {

struct PacketPropertiesAssigner {
    virtual auto assign_value(int bytes_remaining) const -> double = 0;

    PacketPropertiesAssigner() = default;
    PacketPropertiesAssigner(PacketPropertiesAssigner const&) = delete;
    PacketPropertiesAssigner& operator=(PacketPropertiesAssigner const&) = delete;

    virtual ~PacketPropertiesAssigner() = default;
};


}

#endif //DELAY_TRANSPORT_PACKETPROPERTIESASSIGNER_H
