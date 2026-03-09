#include "kernel/keyboard/keyboard.hpp"

#include "asm/instructions.hpp"
#include "kernel/console/console.hpp"

#include <stdint.h>

void HandleKeyBoardIRQ() {
    unsigned char status = inb(0x64);
    if (!(status & 0b00000001)) return; // no data in buffer

    if (status & 0b00100000) { // mouse input
        return;
    }

    unsigned char scanCode = inb(0x60);
    
    constexpr unsigned char PRESS_MIN = 0x01;
    constexpr unsigned char PRESS_MAX = 0x7F;
    constexpr unsigned char RELEASE_MIN = 0x80;

    if (scanCode > PRESS_MIN && scanCode < PRESS_MAX) {    
        kernel::printf("Key pressed: ", static_cast<uint32_t>(scanCode), '\n');
    }
    else {
        kernel::printf("Key released: ", static_cast<uint32_t>(scanCode), '\n');
    }
}
