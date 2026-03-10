#include "kernel/time/time.hpp"

#include <stdint.h>

namespace kernel {
    namespace {
        volatile uint64_t ticksSinceStart = 0;
    }
    // Defined in timer.hpp
    uint32_t callBackNum = 0;
    TickCallback_T callbacks[MAX_TICK_CALLBACKS_NUM] {};

    uint64_t getCurrentTick() {
        return ticksSinceStart;
    }

    void timerTick() {
        ticksSinceStart = ticksSinceStart + 1;

        for (uint32_t i = 0; i < callBackNum; ++i) {
            callbacks[i]();
        }
    }
}
