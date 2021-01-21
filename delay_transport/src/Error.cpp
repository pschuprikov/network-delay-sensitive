#include "delay_transport/Error.h"

namespace delay_transport {

char const *Error::what() const noexcept { return reason.c_str(); }

Error::Error(std::string const &reason)
      : reason("DelayTransport (ERROR): " + reason) {}

} // namespace delay_transport
