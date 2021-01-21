#ifndef DELAY_TRANSPORT_EXPIRATIONLISTENER_H
#define DELAY_TRANSPORT_EXPIRATIONLISTENER_H

#include <delay_transport/ApplicationMessageData.h>

namespace delay_transport {

struct ExpirationListener {
    virtual void report_expiration(ApplicationMessageData const &app,
                                   int bytes_remaining) = 0;

    ExpirationListener() = default;
    ExpirationListener(ExpirationListener const &) = delete;
    ExpirationListener const &operator=(ExpirationListener const &) = delete;

    virtual ~ExpirationListener() = default;
};

} // namespace delay_transport

#endif // DELAY_TRANSPORT_EXPIRATIONLISTENER_H
