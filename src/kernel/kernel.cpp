#include "kernel/kernel.hpp"

#include "kernel/keyboard/keyboard.hpp"
#include "kernel/console/console.hpp"
#include "kernel/interrupt/idt.hpp"
#include "kernel/gdt/gdt.hpp"

#include "kernel/time/time.hpp"

void Kernel::Init() {
    kernel::clearConsole();

    kernel::printf("Initializing kernel...\n");

    kernel::printf("    - Setting up GDT\n");

    gdt_load();

    kernel::printf("    - Setting up IDT\n");

    idt_init();

    kernel::ScopedColor color(kernel::Color::GREEN_ON_BLACK);
    kernel::printf("Successfully initialized kernel!\n");

    kernel::setTickCallbacks(kernel::cursorTick, kernel::KeyBoardTick);
}

void Kernel::Run() {
    this->Init();

    while (true) {
        char c = kernel::keyboard::getChar();
        if (c == '\x1B') {
            uint8_t key;
            if (kernel::keyboard::readEscape(key)) {
                switch (key) {
                    case kernel::keyboard::KEY_LEFT: kernel::moveVisualCursor(kernel::MovementDirection::LEFT); break;
                    case kernel::keyboard::KEY_RIGHT: kernel::moveVisualCursor(kernel::MovementDirection::RIGHT); break;
                }
            }
        } else {
            kernel::printf(c);
        }
    }
}
