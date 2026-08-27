#include "kernel/acpi/sdt.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/error/error.hpp"
#include "kernel/debug/gop.hpp"

#include "kernel/memory/memory-defs.hpp"

SDTHeader* sdtHeader = nullptr;

static void sdtHeader_load(uint64_t address) {
    sdtHeader = reinterpret_cast<SDTHeader*>(TO_VIRT(address));

    bool isXsdt = rsdp.revision >= 2;
    const char* expected = isXsdt ? "XSDT" : "RSDT";

    for (int i = 0; i < 4; ++i) {
        if (sdtHeader->signature[i] != expected[i]) {
            KERNEL_PRINT("        - SDT Signature mismatch!\n");
            sdtHeader = nullptr;
            return;
        }
    }
}

void sdtHeader_load() {
    KERNEL_ASSERT(rsdp.rsdt_address != 0);
    
    uint64_t address = rsdp.revision >= 2
        ? rsdp.xsdt_address
        : static_cast<uint64_t>(rsdp.rsdt_address);
    KERNEL_PRINT("        - SDT Address: ");
    KERNEL_PRINTHEX(address);
    KERNEL_PRINT('\n');
    sdtHeader_load(address);
}
