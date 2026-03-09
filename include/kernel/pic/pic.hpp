#pragma once

#include "stdint.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIT_CMD  0x43
#define PIT_DATA 0x40

void pic_remap();
void pic_eoi(uint8_t irq);
void pit_init(uint32_t hz);
