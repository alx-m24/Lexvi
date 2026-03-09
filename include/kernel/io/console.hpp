#pragma once

namespace kernel {
    void clearConsole();

    inline void printf() {}
    void printf(char c);
    void printf(const char* str);
    void printf(unsigned int n);
    void printf(int n);

    template<typename T, typename... Other>
    void printf(const T& first, const Other&... other) {
        printf(first);
        printf(other...);
    }
}
