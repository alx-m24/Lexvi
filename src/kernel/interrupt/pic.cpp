#include "kernel/interrupt/pic.hpp"

#include "asm/instructions.hpp"

#include "kernel/debug/serial.hpp"

void pic_remap(Interrupts interrupts) {
    kernel::serial::put("       - Remapping PIC\n");

    outb(PIC1_CMD, 0x11);   // ICW1: start init, expect ICW4
    outb(PIC2_CMD, 0x11);
    outb(PIC1_DATA, 0x20);  // ICW2: master offset -> IRQ0-7 map to vectors 0x20-0x27
    outb(PIC2_DATA, 0x28);  // ICW2: slave offset  -> IRQ8-15 map to vectors 0x28-0x2F
    outb(PIC1_DATA, 0x04);  // ICW3: tell master there's a slave on IRQ2
    outb(PIC2_DATA, 0x02);  // ICW3: tell slave its cascade identity
    outb(PIC1_DATA, 0x01);  // ICW4: 8086 mode
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, interrupts.getPICMask(PIC1_DATA));
    outb(PIC2_DATA, interrupts.getPICMask(PIC2_DATA));

    uint8_t data1 = inb(PIC1_DATA);
    uint8_t data2 = inb(PIC2_DATA);

    kernel::serial::put("           Data1: ");
    kernel::serial::putHex(data1);
    kernel::serial::put(" data2: ");
    kernel::serial::putHex(data2);
    kernel::serial::put("\n");
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
