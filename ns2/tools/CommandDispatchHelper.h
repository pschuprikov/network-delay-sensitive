#ifndef NS2_COMMANDDISPATCHHELPER_H
#define NS2_COMMANDDISPATCHHELPER_H

#include <tcl.h>
#include <string_view>
#include <optional>
#include <type_traits>

namespace nfp::command {

enum class TclResult {
    OK = TCL_OK,
    ERROR = TCL_ERROR,
};

template<class T>
struct dispatch_helper {
    dispatch_helper(T * instance, int argc, char const * const * argv)
        : instance{instance}, argc{argc}, argv{argv} {}

    template<class... Args>
    [[nodiscard]]
    auto& add(char const * name, TclResult(T::*method)(Args... args)) {
        return add(name, method, std::index_sequence_for<Args...>());
    }

    template<class R, class... Args>
    [[nodiscard]]
    auto& add(char const *, R(T::*)(Args...)) {
        static_assert(!std::is_same<R, TclResult>::value, "return type should be TclResult");
    }

    template<class Next>
    [[nodiscard]]
    int result(Next next) const {
        if (value) {
            return static_cast<int>(*value);
        } else {
            return next(argc, argv);
        }
    }

private:
    template<class... Args, std::size_t... I>
    [[nodiscard]]
    auto& add(char const * name, TclResult(T::*method)(Args... args), std::index_sequence<I...>) {
        if (!value && argc == sizeof...(Args) + 2 && argv[1] == std::string_view(name)) {
            value = (instance->*method)(argv[2 + I]...);
        }
        return *this;
    }

private:
    T * const instance;
    int const argc;
    char const * const * const argv;
    std::optional<TclResult> value;
};

template<class T>
auto dispatch(T * instance, int argc, char const * const * argv) {
    return dispatch_helper<T>(instance, argc, argv);
}

}

#endif //NS2_COMMANDDISPATCHHELPER_H
