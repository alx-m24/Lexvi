#pragma once

#include "stdint.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIT_CMD  0x43
#define PIT_DATA 0x40

struct Interrupts {
    bool systemClock{};
    bool keyboard{};
    // TODO: Add

    uint8_t getPICMask(uint8_t pic) const {
        uint8_t val = 0xFF;

        if (pic == PIC1_DATA) {
            val &= ~(static_cast<uint8_t>(systemClock) << 0);
            val &= ~(static_cast<uint8_t>(keyboard) << 1);
            val &= ~(static_cast<uint8_t>(1) << 2); // cascade line to PIC2 (for IRQ 8-15)
        }

        return val;
    }
};

void pic_remap(Interrupts interrupts);
void pic_eoi(uint8_t irq);
void pit_init(uint32_t hz);
