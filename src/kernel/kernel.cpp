#include "kernel/kernel.hpp"

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
}

void Kernel::Run() {
    this->Init();
    uint32_t timeSeconds = 0;
    uint64_t nextTick = kernel::getTicksSinceStart() + CLOCK_FREQ;
    while (true) {
        kernel::printf(++timeSeconds, "s\n");
        kernel::busySleepUntil(nextTick);
        nextTick += CLOCK_FREQ;
    }
}
