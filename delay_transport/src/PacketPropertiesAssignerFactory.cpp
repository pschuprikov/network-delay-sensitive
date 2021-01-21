#include <delay_transport/ApplicationMessageData.h>
#include <delay_transport/PacketPropertiesAssignerFactory.h>

namespace {

using namespace delay_transport;

class UniformPacketPropertiesAssigner : public PacketPropertiesAssigner {
  public:
    [[nodiscard]] auto assign_value([[maybe_unused]] int bytes_remaining) const
            -> double override {
        return 1.0;
    }
};

class InvRemSizePacketPropertiesAssigner : public PacketPropertiesAssigner {
  public:
    [[nodiscard]] auto assign_value(int bytes_remaining) const
            -> double override {
        return 1.0 / (bytes_remaining + 1);
    }
};

class InvSizePacketPropertiesAssigner : public PacketPropertiesAssigner {
  public:
    explicit InvSizePacketPropertiesAssigner(int message_length)
        : message_length{message_length} {}

    [[nodiscard]] auto assign_value([[maybe_unused]] int bytes_remaining) const
            -> double override {
        return 1.0 / message_length;
    }

  private:
    int const message_length;
};

class LeastAttainedServicePacketPropertiesAssigner : public PacketPropertiesAssigner {
  public:
    explicit LeastAttainedServicePacketPropertiesAssigner(int message_length)
        : message_length{message_length} {}

    [[nodiscard]] auto assign_value(int bytes_remaining) const
            -> double override {
        return 1.0 / (message_length - bytes_remaining + 1);
    }

  private:
    int const message_length;
};

template <class CreateFun>
class PacketPropertiesAssignerFactoryImpl
    : public PacketPropertiesAssignerFactory {
  public:
    explicit PacketPropertiesAssignerFactoryImpl(CreateFun fun)
        : fun{std::move(fun)} {}

  public:
    [[nodiscard]] auto
    create_instance(ApplicationMessageData const &app_message) const
            -> std::unique_ptr<PacketPropertiesAssigner> override {
        return fun(app_message);
    }

  private:
    CreateFun const fun;
};

template <class CreateFun> auto create_factory(CreateFun fun) {
    return std::make_unique<PacketPropertiesAssignerFactoryImpl<CreateFun>>(
            fun);
}

} // namespace

namespace delay_transport {

auto PacketPropertiesAssignerFactory::create_from_json(
        nlohmann::json json, [[maybe_unused]] Environment *env)
        -> std::unique_ptr<PacketPropertiesAssignerFactory> {
    if (json["type"] == "uniform") {
        return create_factory([](auto const &) {
            return std::make_unique<UniformPacketPropertiesAssigner>();
        });
    }

    if (json["type"] == "inv_size") {
        return create_factory([](auto const &app_message) {
            return std::make_unique<InvSizePacketPropertiesAssigner>(
                    app_message.length_in_bytes);
        });
    }
    if (json["type"] == "inv_rem_size") {
        return create_factory([](auto const &) {
            return std::make_unique<InvRemSizePacketPropertiesAssigner>();
        });
    }
    if (json["type"] == "las") {
        return create_factory([](auto const &app_message) {
            return std::make_unique<LeastAttainedServicePacketPropertiesAssigner>(
                    app_message.length_in_bytes);
        });
    }

    return nullptr;
}

} // namespace delay_transport
