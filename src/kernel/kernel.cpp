#include "kernel/kernel.hpp"

#include "kernel/memory/pmm.hpp"

#include "kernel/keyboard/keyboard.hpp"
#include "kernel/console/tui/window.hpp"
#include "kernel/console/console.hpp"
#include "kernel/interrupt/idt.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/acpi/sdt.hpp"
#include "kernel/gdt/gdt.hpp"


#include "kernel/time/time.hpp"

void Kernel::Init() {
    kernel::clearConsole();

    kernel::printf("Initializing kernel...\n");

    kernel::printf("    - Setting up GDT\n");
    gdt_load();

    kernel::printf("    - Setting up IDT\n");
    idt_init();

    kernel::printf("    - Setting up RSDP\n");
    rsdp_load();

    PMM pmm;
    pmm.Init();

    // kernel::printf("    - Setting up SDT Header\n");
    // sdtHeader_load();

    {
        kernel::ScopedColor color(kernel::Color::GREEN_ON_BLACK);
        kernel::printf("Successfully initialized kernel!\n");
    }

    kernel::setTickCallbacks(kernel::cursorTick, kernel::KeyBoardTick);


    kernel::Window mainWindow{};
    mainWindow.clear();
    
    mainWindow.printf("Hi from main window\n");
    mainWindow.printf("This is a small test of the new console size", '\n', '\t', "Hopefully all this code works well\n", 69.67, '\n', 789u, '\n', -10, '\n', -897.5, '\n');

    while (true);
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
