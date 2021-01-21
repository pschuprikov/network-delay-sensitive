#ifndef NS2_EXPIRATION_TRACKING_MIXIN_H
#define NS2_EXPIRATION_TRACKING_MIXIN_H

#include <delay_cc/queue/utils/ExpirationTracker.h>
#include <type_traits>

template<class Base>
class ExpirationTrackingMixin : public Base {
public:
    void enque(Packet *p) override;
    void drop(Packet * packet) override;
    auto deque() -> Packet * override;

    template<class R> auto remove_expired(R remover) const -> bool;

private:
    ExpirationTracker expiration_tracker_;
};

#endif // NS2_EXPIRATION_TRACKING_MIXIN_H
