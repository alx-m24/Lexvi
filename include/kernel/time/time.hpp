#pragma once

#define CLOCK_FREQ 1000 // hertz

#include <stdint.h>

namespace kernel {
    void timerTick();
    
    void busySleepUntil(uint64_t tick);
    void busySleepMs(uint32_t ms);
    void busySleep_seconds(uint32_t seconds);

    uint64_t getTicksSinceStart();
}
