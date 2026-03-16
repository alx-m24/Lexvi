#include "kernel/kernel.hpp"

#include "kernel/memory/pmm.hpp"

#include "kernel/keyboard/keyboard.hpp"
#include "kernel/console/tui/window.hpp"
#include "kernel/console/console.hpp"
#include "kernel/interrupt/idt.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/acpi/sdt.hpp"
#include "kernel/gdt/gdt.hpp"

#include "kernel/console/tui/rectangle.hpp"
#include "kernel/console/tui/outline.hpp"

#include "kernel/time/time.hpp"

kernel::Window mainWindow {};
kernel::Window inputWindow {};
kernel::Window logWindow {};

static void blinkInputWindowCursorTick() {
    inputWindow.BlinkCursor();
}

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

    mainWindow.clear();

    inputWindow = kernel::Window(&mainWindow, mainWindow.getWidth() / 2, mainWindow.getHeight(), 0, 0);
    logWindow = kernel::Window(&mainWindow, mainWindow.getWidth() / 2, mainWindow.getHeight(), mainWindow.getWidth() / 2, 0);

    kernel::setLogWindow(&logWindow);
    kernel::setTickCallbacks(blinkInputWindowCursorTick, kernel::KeyBoardTick);
    
    inputWindow.Draw(kernel::Outline(0, 0, inputWindow.getWidth(), inputWindow.getHeight()));
    inputWindow.setPrintableArea(1, 1, inputWindow.getWidth() - 1, inputWindow.getHeight() - 1);

    logWindow.Draw(kernel::Outline(0, 0, logWindow.getWidth(), logWindow.getHeight()));
    logWindow.setPrintableArea(1, 1, logWindow.getWidth() - 1, logWindow.getHeight() - 1);

    logWindow.printf("Hi from log window\n");
    logWindow.printf("This is a small test of the new console size", '\n', '\t', "Hopefully all this code works well\n", 69.67, '\n', 789u, '\n', -10, '\n', -897.5, '\n');

}

void Kernel::Run() {
    this->Init();

    while (true) {
        char c = kernel::keyboard::getChar();
        if (c == '\x1B') {
            uint8_t key;
            if (kernel::keyboard::readEscape(key)) {
                switch (key) {
                    case kernel::keyboard::KEY_LEFT: inputWindow.moveVisualCursor(kernel::MovementDirection::LEFT); break;
                    case kernel::keyboard::KEY_RIGHT: inputWindow.moveVisualCursor(kernel::MovementDirection::RIGHT); break;
                }
            }
        } else {
            inputWindow.printf(c);
        }
    }
}
