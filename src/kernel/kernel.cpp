#include "kernel/kernel.hpp"

#include "kernel/memory/pmm.hpp"
#include "kernel/memory/memory-window.hpp"

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

kernel::Window LeftSplit {};
kernel::Window logWindow {};
kernel::MemoryWindow memoryWindow {};

static void blinkInputWindowCursorTick() {
    inputWindow.BlinkCursor();
}

void Kernel::Init() {
    {   // console stuff
        kernel::clearConsole();
        
        mainWindow.clear();

        LeftSplit = kernel::Window(&mainWindow, mainWindow.getWidth() / 2, mainWindow.getHeight(), 0, 0);

        logWindow = kernel::Window(&mainWindow, mainWindow.getWidth() / 2, mainWindow.getHeight(), mainWindow.getWidth() / 2, 0);

        inputWindow = kernel::Window(&LeftSplit, LeftSplit.getWidth(), LeftSplit.getHeight() / 2, 0, 0);
        memoryWindow = kernel::MemoryWindow(&LeftSplit, LeftSplit.getWidth(), LeftSplit.getHeight() / 2 - 1, 0, LeftSplit.getHeight() / 2);

        kernel::setLogWindow(&logWindow);

        inputWindow.Draw(kernel::Outline(0, 0, inputWindow.getWidth(), inputWindow.getHeight()));
        {
            auto scopedColor = inputWindow.setScopedColor(kernel::Color::RED_ON_BLACK);
            inputWindow.Draw(kernel::Rectangle(1, 1, inputWindow.getWidth() - 2, inputWindow.getHeight() - 2));
        }
        inputWindow.setPrintableArea(1, 1, inputWindow.getWidth() - 1, inputWindow.getHeight() - 1);

        logWindow.Draw(kernel::Outline(0, 0, logWindow.getWidth(), logWindow.getHeight()));
        logWindow.setPrintableArea(1, 1, logWindow.getWidth() - 1, logWindow.getHeight() - 1);

        logWindow.printf("Hi from log window\n");
        logWindow.printf("This is a small test of the new console size", '\n', '\t', "Hopefully all this code works well\n", 69.67, '\n', 789u, '\n', -10, '\n', -897.5, '\n');
    }

    kernel::printf("Initializing kernel...\n");
    
    kernel::printf("    - Setting up pmm\n");
    kernel::PMM pmm;
    pmm.Init(&memoryWindow);

    kernel::printf("    - Setting up GDT\n");
    gdt_load();

    kernel::printf("    - Setting up IDT\n");
    idt_init();
    kernel::setTickCallbacks(blinkInputWindowCursorTick, kernel::KeyBoardTick);

    kernel::printf("    - Setting up RSDP\n");
    rsdp_load();

    {
        kernel::ScopedColor color(kernel::Color::GREEN_ON_BLACK);
        kernel::printf("Successfully initialized kernel!\n");
    }
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
