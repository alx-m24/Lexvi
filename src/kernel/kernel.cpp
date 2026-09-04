#include "kernel/kernel.hpp"

#include "kernel/memory/memory-defs.hpp"

#include "kernel/debug/gop.hpp"

#include "kernel/acpi/pci/pmc/reset_cause.hpp"
#include "kernel/acpi/pci/p2sb/p2sbc.hpp"
#include "kernel/acpi/pci/SMBus/tco.hpp"
#include "kernel/keyboard/keyboard.hpp"
#include "kernel/interrupt/idt.hpp"
#include "kernel/acpi/mcfg.hpp"
#include "kernel/acpi/fadt.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/acpi/hpet.hpp"
#include "kernel/acpi/sdt.hpp"
#include "kernel/gdt/gdt.hpp"

#include "kernel/time/time.hpp"

#include "kernel/acpi/pci/pci-test.hpp"

void Kernel::Init() {
    memoryManager.Init();

    kernel::load_GOP();
    memoryManager.m_vmm.mapMMIO(MMIO_TO_VIRT(kernel::gop.FrameBufferBase), kernel::gop.FrameBufferBase, Bytes(kernel::gop.FrameBufferSize));
    kernel::init_FrameBuffer();
    kernel::gop_fill(kernel::GOP::Color{ 0, 0, 0 });

    memoryManager.TestMemory();
 
    KERNEL_PRINT("Initializing kernel...\n");

    KERNEL_PRINT("    - Setting up GDT\n");
    gdt_load();

    KERNEL_PRINT("    - Setting up IDT\n");
    idt_init();
    kernel::setTickCallbacks(kernel::KeyBoardTick);

    KERNEL_PRINT("    - Setting up RSDP\n");
    rsdp_load();

    KERNEL_PRINT("    - Setting up SDT\n");
    sdtHeader_load();

    KERNEL_PRINT("      - Setting up MCFG\n");
    mcfg_load(memoryManager.m_vmm);

    KERNEL_PRINT("      - Testing PCI reads\n");
    kernel::tests::testPCIRead();

    KERNEL_PRINT("  - Setting up FADT\n");
    loadFADT();
    // kernel::GOP::reset(); log_fadt();

    KERNEL_PRINT("      - Testing PCI writes\n");
    kernel::tests::pciWriteTestRoutine();

    kernel::GOP::reset();
    KERNEL_PRINT("      - Unhidding P2SB\n");
    if (kernel::P2SB::unhide_p2sb()) {
        KERNEL_PRINT("      - P2SB successfully unhidden\n");
        kernel::P2SB::hide_p2sb();
    }
    else {
        KERNEL_PRINT("\n=== FAILED TO UNHIDE P2SB ===\n\n");
    }

    if (kernel::SMBus::disableTCO()) {
        KERNEL_PRINT("      - TCO halted successfully\n");
    }
    else {
        KERNEL_PRINT("\n=== FAILED TO HALT TCO ===\n\n");
    }

    {
        kernel::P2SB::ScopedP2SBCUnhide scopedUnhide{};
        kernel::PMC::getResetCause(memoryManager.m_vmm);
    }

    KERNEL_PRINT("    - Setting up HPET\n");
    hpet_load();
    memoryManager.m_vmm.mapMMIO(MMIO_TO_VIRT(hpet_base), hpet_base, KiB(4_KiB).bytes());

    KERNEL_PRINT("    - Setting up Chrono\n");
    kernel::chrono::init();

    KERNEL_PRINT("\nSuccessfully initialized kernel!\n");
 }

void Kernel::Run() {
    this->Init();

    kernel::GOP::print("=== Kernel Running ===", " \nKeyboard input: ");

    while (true) {
        char c = kernel::keyboard::getChar();
        if (c == '\x1B') {
            uint8_t key;
            if (kernel::keyboard::readEscape(key)) {
                switch (key) {
                    // TODO
                }
            }
        } else {
            if (c != '\0') kernel::GOP::print(c);
        }

        // kernel::GOP::print("Time: ", kernel::chrono::now().to<kernel::chrono::Unit::Milliseconds>().value, "ms\n");
    }
}
