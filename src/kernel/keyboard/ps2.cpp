#include "kernel/keyboard/ps2.hpp"

#include "asm/instructions.hpp"
#include <cstdint>

void ps2_init() {
    // 1. Disable PS/2 devices during setup
    outb(0x64, 0xAD); // Disable first PS/2 port (keyboard)
    outb(0x64, 0xA7); // Disable second PS/2 port (mouse)

    // 2. Flush residual bytes in buffer
    while (inb(0x64) & 1) {
        inb(0x60);
    }

    // 3. Read Controller Command Byte
    outb(0x64, 0x20);
    while (!(inb(0x64) & 1));
    uint8_t config = inb(0x60);

    // 4. Enable IRQ1 (Bit 0) & Keyboard Clock Line (Clear Bit 4)
    config |= (1 << 0);  // Enable First PS/2 Port Interrupt
    config &= ~(1 << 4); // Enable First PS/2 Port Clock line
    config &= ~(1 << 1); // Disable mouse IRQ12 for now

    // 5. Write updated Command Byte back
    outb(0x64, 0x60);
    while (inb(0x64) & 2); // Wait for input buffer to be ready
    outb(0x60, config);

    // 6. Re-enable First PS/2 Port
    outb(0x64, 0xAE);

    // 7. Send "Enable Scanning" command (0xF4) directly to Keyboard device
    outb(0x60, 0xF4);
    
    // Wait for ACK (0xFA) or response and drain it
    for (int i = 0; i < 1000; i++) {
        if (inb(0x64) & 1) {
            uint8_t ack = inb(0x60);
            (void)ack;
            break;
        }
    }
}
