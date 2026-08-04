#include "kernel/time/time.hpp"

#include <stdint.h>
#include <cstdint>

#include "kernel/debug/serial.hpp"
#include "kernel/acpi/hpet.hpp"
#include "kernel/memory/memory-defs.hpp"

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

    namespace chrono {
        volatile uint64_t* hpet_regs = nullptr;
        uint64_t period_fs = 0;
    }

    void chrono::init() {
        hpet_regs = reinterpret_cast<volatile uint64_t*>(MMIO_TO_VIRT(hpet_base));
        period_fs = hpet_regs[0] >> 32;
        kernel::serial::put("Here\n");
        hpet_regs[2] = hpet_regs[2] | 1;
    }
}
