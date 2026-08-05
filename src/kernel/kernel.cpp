#include "kernel/kernel.hpp"

#include "kernel/memory/memory-defs.hpp"

#include "kernel/debug/serial.hpp"
#include "kernel/debug/gop.hpp"

#include "kernel/keyboard/keyboard.hpp"
#include "kernel/interrupt/idt.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/acpi/hpet.hpp"
#include "kernel/acpi/sdt.hpp"
#include "kernel/gdt/gdt.hpp"

#include "kernel/time/time.hpp"

void Kernel::Init() {
    kernel::serial::put("Reloading Memory Manager...\n");
    memoryManager.Init();
    memoryManager.TestMemory();

    kernel::serial::put("Initializing kernel...\n");

    kernel::serial::put("    - Setting up GDT\n");
    gdt_load();

    kernel::serial::put("    - Setting up IDT\n");
    idt_init();
    kernel::setTickCallbacks(kernel::KeyBoardTick);

    kernel::serial::put("    - Setting up RSDP\n");
    rsdp_load();

    kernel::serial::put("    - Setting up SDT\n");
    sdtHeader_load();

    kernel::serial::put("    - Setting up HPET\n");
    hpet_load();
    memoryManager.m_vmm.mapMMIO(MMIO_TO_VIRT(hpet_base), hpet_base, KiB(4_KiB).bytes());

    kernel::serial::put("    - Setting up Chrono\n");
    kernel::chrono::init();
        
    kernel::serial::put("    - Setting up GOP\n");
    kernel::load_GOP();
    kernel::serial::put("    - Mapping GOP\n");
    memoryManager.m_vmm.mapMMIO(MMIO_TO_VIRT(kernel::gop.FrameBufferBase), kernel::gop.FrameBufferBase, Bytes(kernel::gop.FrameBufferSize));
    kernel::init_FrameBuffer();
    kernel::serial::put("    - Testing GOP\n");
    kernel::gop_test();

    kernel::serial::put("\nSuccessfully initialized kernel!\n");
 }

void Kernel::Run() {
    this->Init();
    
    kernel::serial::put("\n\n === Kernel Running ===", " \nKeyboard input: ");

    while (true) {
        char c = kernel::keyboard::getChar();
        if (c == '\x1B') {
            uint8_t key;
            if (kernel::keyboard::readEscape(key)) {
                switch (key) {
                    // TODO
                }
            }
        } else if (c != '\0') {
            kernel::serial::put(c);
        }

        // kernel::serial::put("Time: ", kernel::chrono::now().to<kernel::chrono::Unit::Milliseconds>().value, "ms\n");
    }
}
