#pragma once

#include <cstdint>

namespace kernel {
    inline bool equals(const char* a, const char* b) {
        uint64_t i = 0;
        while (a[i] != '\0' && b[i] != '\0') {
            if (a[i] != b[i]) return false;
            ++i;
        }
        return a[i] == b[i];
    }

    inline bool equalsN(const char* a, const char* b, uint64_t n) {
        for (uint64_t i = 0; i < n; ++i)
            if (a[i] != b[i]) return false;
        return true;
    }
}
