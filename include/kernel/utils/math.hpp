#pragma once

namespace kernel {
    template<typename T>
    T max(T a, T b) {
        if (a > b) return a;
        return b;
    }

    template<typename First, typename... Others>
    First max(First a, const Others&... others) {
        return max(a, max(others...));
    }
}
