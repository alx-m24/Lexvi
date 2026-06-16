#pragma once

#include <functional>
#include <type_traits>
#include <cstdint>
#include "kernel/acpi/hpet.hpp"
#include "kernel/memory/memory-defs.hpp"

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


    namespace chrono {
        enum class Unit : uint64_t {
            Femtoseconds = 1,
            Nanoseconds  = 1000000,
            Microseconds = 1000000000,
            Milliseconds = 1000000000000,
            Seconds      = 1000000000000000,
        };

        template<Unit U>
        struct Duration {
            uint64_t value;
            explicit Duration(uint64_t v) : value(v) {}

            template<Unit To>
            Duration<To> to() const {
                return Duration<To>(value * static_cast<uint64_t>(U) / static_cast<uint64_t>(To));
            }
        };

        extern volatile uint64_t* hpet_regs;
        extern uint64_t period_fs;

        void init();

        template<Unit U = Unit::Nanoseconds>
        Duration<U> now() {
            return Duration<U>(hpet_regs[30] * period_fs / static_cast<uint64_t>(U));
        }
    }
}
