#ifndef NS2_BIND_CACHING_MIXING
#define NS2_BIND_CACHING_MIXING

#include <tclcl.h>
#include <memory>

template<class Base>
class BindCachingMixin : public Base {
public:
    template<class... Args>
    BindCachingMixin(Args&&...);

    ~BindCachingMixin() override;

protected:
    void delay_bind_init_all() override;
    int delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer) override;

    void delay_bind_init_one(const char * varName);

	bool delay_bind_bool(const char *varName, const char* localName, const char* thisVarName, int* val, TclObject *tracer);
    bool delay_bind(const char *varName, const char* localName, const char* thisVarName, int* val, TclObject *tracer);
    bool delay_bind(const char *varName, const char* localName, const char* thisVarName, double* val, TclObject *tracer);
    bool delay_bind(const char *varName, const char* localName, const char* thisVarName, TracedInt* val, TclObject *tracer);

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl_;
};

#endif // NS2_BIND_CACHING_MIXING
