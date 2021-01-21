#include <DelayTransportImpl.h>
#include <delay_transport/DelayTransport.h>

namespace delay_transport {
auto DelayTransport::create_instance(
        Environment *environment,
        std::shared_ptr<PacketPropertiesAssignerFactory> packet_properties)
        -> std::unique_ptr<DelayTransport> {
    return std::make_unique<DelayTransportImpl>(environment, packet_properties);
}

} // namespace delay_transport
