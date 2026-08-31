#pragma once

#include <type_traits>

namespace kernel {
    template<typename A, typename B>
    constexpr auto max(A a, B b) {
        using Common = std::common_type_t<A, B>;
        return static_cast<Common>(a > b ? a : b);
    }

    template<typename First, typename... Others>
    requires (sizeof...(Others) > 1)
    constexpr auto max(First a, const Others&... others) {
        return max(a, max(others...));
    }

    template<auto a, auto b>
    consteval auto max() {
        if constexpr (a > b) return a;
        return b;
    }

    template<auto a, auto... b>
    requires (sizeof...(b) > 1)
    consteval auto max() {
        return max<a, max<b...>()>();
    }
}
