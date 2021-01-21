#ifndef DELAY_TRANSPORT_PACKETPROPERTIESASSIGNERFACTORY_H
#define DELAY_TRANSPORT_PACKETPROPERTIESASSIGNERFACTORY_H

#include <delay_transport/Environment.h>
#include <delay_transport/PacketPropertiesAssigner.h>
#include <nlohmann/json.hpp>
#include "ApplicationMessageData.h"

namespace delay_transport {

struct PacketPropertiesAssignerFactory {
    static auto create_from_json(nlohmann::json json, Environment * env) -> std::unique_ptr<PacketPropertiesAssignerFactory>;

    [[nodiscard]]
    virtual auto create_instance(const delay_transport::ApplicationMessageData &app_message) const -> std::unique_ptr<PacketPropertiesAssigner> = 0;

    PacketPropertiesAssignerFactory() = default;

    PacketPropertiesAssignerFactory(PacketPropertiesAssignerFactory const&) = delete;
    PacketPropertiesAssignerFactory& operator=(PacketPropertiesAssignerFactory const&) = delete;

    virtual ~PacketPropertiesAssignerFactory() = default;
};

}
#endif //DELAY_TRANSPORT_PACKETPROPERTIESASSIGNERFACTORY_H
