#include "kernel/kernel.hpp"

#include "kernel/memory/memory-defs.hpp"

#include "kernel/debug/gop.hpp"

#include "kernel/acpi/pci/vendor_deviceID.hpp"
#include "kernel/keyboard/keyboard.hpp"
#include "kernel/acpi/pci/p2sb.hpp"
#include "kernel/interrupt/idt.hpp"
#include "kernel/acpi/pci/pwmr.hpp"
#include "kernel/acpi/pci/tco.hpp"
#include "kernel/acpi/mcfg.hpp"
#include "kernel/acpi/fadt.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/acpi/hpet.hpp"
#include "kernel/acpi/sdt.hpp"
#include "kernel/gdt/gdt.hpp"

#include "kernel/time/time.hpp"

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

    KERNEL_PRINT("   - Getting Device ID\n");
    kernel::DeviceID deviceID{};
    kernel::VendorID vendorID{};
    kernel::getVendorDeviceID(&vendorID, &deviceID);
    KERNEL_PRINT("      - Vendor: ", vendorID.getVendorName(), "\n");
    KERNEL_PRINT("      - Device: ");
    KERNEL_PRINTHEX(deviceID.getDeviceID());
    KERNEL_PRINT('\n');

    KERNEL_PRINT("  - Setting up FADT\n");
    loadFADT();
    // kernel::GOP::reset(); log_fadt();

    if (kernel::pciTestWrite()) {
        KERNEL_PRINT("Writes on PCI successfull\n"); 
    }
    else {
        KERNEL_PRINT("Writes on PCI fails\n"); 
    }

    KERNEL_PRINT("      - Unhidding P2SB\n");
    if (kernel::unhide_p2sb()) {

        KERNEL_PRINT("      - P2SB successfully unhidden\n");
    }
    else {
        KERNEL_PRINT("\n=== FAILED TO UNHIDE P2SB ===\n\n");
    }

    if (kernel::disableTCO()) {
        KERNEL_PRINT("      - TCO halted successfully\n");
    }
    else {
        KERNEL_PRINT("\n=== FAILED TO HALT TCO ===\n\n");
    }

    kernel::getResetCause(memoryManager.m_vmm);

    KERNEL_PRINT("    - Setting up HPET\n");
    hpet_load();
    memoryManager.m_vmm.mapMMIO(MMIO_TO_VIRT(hpet_base), hpet_base, KiB(4_KiB).bytes());

    KERNEL_PRINT("    - Setting up Chrono\n");
    kernel::chrono::init();

    KERNEL_PRINT("\nSuccessfully initialized kernel!\n");
 }

void Kernel::Run() {
    this->Init();

    KERNEL_PRINT("\n\n === Kernel Running ===", " \nKeyboard input: ");

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
