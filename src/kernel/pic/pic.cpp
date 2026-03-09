#include "kernel/pic/pic.hpp"

#include "asm/instructions.hpp"

void pic_remap() {
    // Save masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // Start init sequence (cascade mode)
    outb(PIC1_CMD,  0x11);
    outb(PIC2_CMD,  0x11);

    // Set vector offsets
    outb(PIC1_DATA, 0x20); // IRQ0 → vector 32
    outb(PIC2_DATA, 0x28); // IRQ8 → vector 40

    // Tell PICs about each other
    outb(PIC1_DATA, 0x04); // PIC1: IRQ2 has slave
    outb(PIC2_DATA, 0x02); // PIC2: cascade identity

    // 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Restore masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_CMD, 0x20); // also tell slave
    outb(PIC1_CMD, 0x20);
}

void pit_init(uint32_t hz) {
    uint32_t divisor = 1193182 / hz; // PIT base frequency is 1.193182 MHz

    outb(PIT_CMD, 0x36);                        // Channel 0, lobyte/hibyte, square wave
    outb(PIT_DATA, divisor & 0xFF);             // Low byte
    outb(PIT_DATA, (divisor >> 8) & 0xFF);      // High byte
}
