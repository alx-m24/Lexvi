#pragma once

#include <type_traits>

namespace kernel {
    template<typename T>
    requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
    T max(T a, T b) {
        if (a > b) return a;
        return b;
    }

    template<typename First, typename... Others>
    First max(First a, const Others&... others) {
        return max(a, max(others...));
    }
}
