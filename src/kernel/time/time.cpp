#include "kernel/time/time.hpp"

#include <stdint.h>

namespace kernel {
    namespace {
        volatile uint64_t ticksSinceStart = 0;
    }


    uint64_t getTicksSinceStart() {
        return ticksSinceStart;
    }
    
    void busySleepUntil(uint64_t tick) {
        while (ticksSinceStart < tick);
    }

    void timerTick() {
        ticksSinceStart = ticksSinceStart + 1;
    }
    
    void busySleepMs(uint32_t ms) {
        busySleepUntil(ticksSinceStart + (ms * CLOCK_FREQ / 1000));
    }
    
    void busySleep_seconds(uint32_t seconds) {
        busySleepMs(seconds * 1000u);
    }
}
