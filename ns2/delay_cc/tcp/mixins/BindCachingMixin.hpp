#include "BindCachingMixin.h"
#include <iostream>
#include <tuple>
#include <unordered_map>


template<class Base>
struct BindCachingMixin<Base>::Pimpl {
    template <class T>
        using CacheKind = std::unordered_map<std::string_view, T>;

    using Cache = std::tuple<CacheKind<int>, CacheKind<double>>;

    template<int i = 0>
    bool has_in_cache(std::string_view s) const {
        if constexpr (i == std::tuple_size_v<Cache> - 1) {
            return false;
        } else {
            return std::get<i>(init_cache).count(s) || has_in_cache<i + 1>(s);
        }
    }

    template<class CacheType, class T, class DispatchF, class TrueBindF>
    bool delay_bind(
            const char *varName, 
            const char* thisVarName, 
            T * val, 
            [[maybe_unused]] DispatchF dispatch, 
            TrueBindF true_bind) {
        using K = CacheKind<CacheType>;

        if (std::string_view(varName) == "__fake__") {
            *val = std::get<K>(init_cache).at(thisVarName);
            return false;
        }

        auto result = true_bind();

        if (result && !std::get<K>(init_cache).count(thisVarName)) {
            std::get<K>(init_cache)[thisVarName] = *val;
        }

        return result;
    }

    static Cache init_cache;
    static bool initialized;
};

template<class Base>
typename BindCachingMixin<Base>::Pimpl::Cache BindCachingMixin<Base>::Pimpl::init_cache{};

template<class Base>
bool BindCachingMixin<Base>::Pimpl::initialized{false};

template<class Base>
BindCachingMixin<Base>::~BindCachingMixin() = default;

template<class Base>
template<class... Args>
BindCachingMixin<Base>::BindCachingMixin(Args&&... args) 
    : Base(std::forward<Args>(args)...)
    , pimpl_(new Pimpl{}) {
}

template<class Base>
void BindCachingMixin<Base>::delay_bind_init_all() {
    if (pimpl_->initialized) {
        delay_bind_dispatch("__fake__", "__fake__", nullptr);
    } else {
        pimpl_->initialized = true;
    }
    Base::delay_bind_init_all();
}

template<class Base>
int BindCachingMixin<Base>::delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer) {
    if (std::string_view(varName) == "__fake__") {
        return TCL_OK;
    }
    return Base::delay_bind_dispatch(varName, localName, tracer);
}

template<class Base>
void BindCachingMixin<Base>::delay_bind_init_one(const char * varName) {
    if (pimpl_->initialized) {
        return;
    }
    Base::delay_bind_init_one(varName);
}

template<class Base>
bool BindCachingMixin<Base>::delay_bind(const char *varName, const char* localName, const char* thisVarName, int* val, TclObject *tracer) {
    return pimpl_->template delay_bind<int>(
            varName, thisVarName, val, [=] { 
                return Base::delay_bind_dispatch(varName, localName, tracer); 
            }, [=] { 
                return Base::delay_bind(
                        varName, localName, thisVarName, val, tracer); 
            });
}

template<class Base>
bool BindCachingMixin<Base>::delay_bind(const char *varName, const char* localName, const char* thisVarName, TracedInt* val, TclObject *tracer) {
    return pimpl_->template delay_bind<int>(
            varName, thisVarName, val, [=] {
                return Base::delay_bind_dispatch(varName, localName, tracer); 
            }, [=] {
                return Base::delay_bind(
                        varName, localName, thisVarName, val, tracer);
            });
}

template<class Base>
bool BindCachingMixin<Base>::delay_bind(const char *varName, const char* localName, const char* thisVarName, double* val, TclObject *tracer) {
    return pimpl_->template delay_bind<double>(
            varName, thisVarName, val, [=] {
                return Base::delay_bind_dispatch(varName, localName, tracer); 
            }, [=] {
                return Base::delay_bind(
                        varName, localName, thisVarName, val, tracer);
            });
}

template<class Base>
bool BindCachingMixin<Base>::delay_bind_bool(const char *varName, const char* localName, const char* thisVarName, int* val, TclObject *tracer) {
    return pimpl_->template delay_bind<int>(
            varName, thisVarName, val, [=] {
                return Base::delay_bind_dispatch(varName, localName, tracer); 
            }, [=] {
                return Base::delay_bind_bool(
                        varName, localName, thisVarName, val, tracer);
            });
}
