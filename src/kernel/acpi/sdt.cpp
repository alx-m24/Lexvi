#include "kernel/acpi/sdt.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/error/error.hpp"
#include "kernel/console/console.hpp"

SDTHeader* sdtHeader = nullptr;

static void sdtHeader_load(uint64_t address) {
    sdtHeader = reinterpret_cast<SDTHeader*>(address);

    bool isXsdt = rsdp.revision >= 2;
    const char* expected = isXsdt ? "XSDT" : "RSDT";

    for (int i = 0; i < 4; ++i) {
        if (sdtHeader->signature[i] != expected[i]) {
            kernel::printf("        - SDT Signature mismatch!\n");
            return;
        }
    }

    kernel::printf("        - SDT Signature: ");
    for (int i = 0; i < 4; ++i) kernel::printf(sdtHeader->signature[i]);
    kernel::printf('\n');
    kernel::printf("        - SDT Length: ", sdtHeader->length, '\n');

    uint32_t pointer_size = isXsdt ? 8 : 4;
    uint32_t entries = (sdtHeader->length - sizeof(SDTHeader)) / pointer_size;

    kernel::printf("        - SDT Entries: ", entries, '\n');

    uint8_t* entry_ptr = reinterpret_cast<uint8_t*>(address + sizeof(SDTHeader));
    for (uint32_t i = 0; i < entries; ++i) {
        uint64_t entry_addr = isXsdt
            ? *reinterpret_cast<uint64_t*>(entry_ptr + i * 8)
            : *reinterpret_cast<uint32_t*>(entry_ptr + i * 4);

        SDTHeader* entry = reinterpret_cast<SDTHeader*>(entry_addr);
        kernel::printf("            - [", static_cast<uint32_t>(i), "] ");
        for (int j = 0; j < 4; ++j) kernel::printf(entry->signature[j]);
        kernel::printf('\n');
    }
}

void sdtHeader_load() {
    KERNEL_ASSERT(rsdp.rsdt_address != 0);
    
    uint64_t address = rsdp.revision >= 2
        ? rsdp.xsdt_address
        : static_cast<uint64_t>(rsdp.rsdt_address);
    kernel::printf("        - SDT Address: ");
    kernel::printfHex(address);
    kernel::printf('\n');
    sdtHeader_load(address);
}
