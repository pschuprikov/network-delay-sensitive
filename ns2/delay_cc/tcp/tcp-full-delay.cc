#include <delay_cc/tcp/DFullTcpAgent.hpp>

static class DSackFullTcpClass : public TclClass {
public:
    DSackFullTcpClass() : TclClass("Agent/TCP/FullTcp/Sack/D") {}
    TclObject* create(int, const char*const*) {
        return (new DFullTcpAgent<SackFullTcpAgent>());
    }
} class_sack_full_d;

static class DMinTcpClass : public TclClass {
public:
    DMinTcpClass() : TclClass("Agent/TCP/FullTcp/Sack/MinTCP/D") {}
    TclObject* create(int, const char*const*) {
        return (new DFullTcpAgent<MinTcpAgent>());
    }
} class_min_full_d;

static class DDDTcpClass : public TclClass {
public:
    DDDTcpClass() : TclClass("Agent/TCP/FullTcp/Sack/DDTCP/D") {}
    TclObject* create(int, const char*const*) {
        return (new DFullTcpAgent<DDTcpAgent>());
    }
} class_dd_full_d;
