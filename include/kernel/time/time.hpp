#pragma once

#include <stdint.h>
#include <functional>
#include <type_traits>

#include "kernel/error/error.hpp"

namespace kernel {
    constexpr uint32_t CLOCK_FREQ = 1000; // hertz
    constexpr uint32_t MAX_TICK_CALLBACKS_NUM = 64;
    
    using TickCallback_T = void (*)();
    extern TickCallback_T callbacks[MAX_TICK_CALLBACKS_NUM];
    extern uint32_t callBackNum;

    void timerTick();

    uint64_t getCurrentTick();

    inline void setTickCallbacks() {} // base case for empty pack

    template<typename First>
    requires std::is_same_v<TickCallback_T, First>
    void setTickCallbacks(const First& first) {
        KERNEL_ASSERT(callBackNum < MAX_TICK_CALLBACKS_NUM - 1);
        callbacks[callBackNum++] = first;
    }

    template<typename First, typename... Other>
    void setTickCallbacks(const First& first, const Other&... others) {
        KERNEL_ASSERT(callBackNum < MAX_TICK_CALLBACKS_NUM - 1);
        callbacks[callBackNum++] = first;
        setTickCallbacks(others...);
    }
}
