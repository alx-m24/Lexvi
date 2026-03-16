#include "kernel/memory/pmm.hpp"

#include "kernel/kernel-config.hpp"
#include "kernel/console/console.hpp"

#include <stdint.h>

extern "C" char _kernel_end[];

struct E820Entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_attrs;
} __attribute__((packed));

void PMM::Init() {
    const uint64_t KERNEL_END_ADDRESS = reinterpret_cast<uint64_t>(_kernel_end);

    const uint32_t MEMORY_MAP_ENTRY_COUNT = *reinterpret_cast<uint32_t*>(MEMORY_MAP_ENTRY_COUNT_ADDRESS);
    E820Entry* entries_ptr = reinterpret_cast<E820Entry*>(MEMORY_MAP_ADDRESS);
    kernel::clearConsole();

    kernel::printf("Memory Map Entries: \n");
    for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i, ++entries_ptr) {
        E820Entry entry = *entries_ptr;
        kernel::printf("    - Entry", i, ": ");
        kernel::printf("Base="); kernel::printfHex(static_cast<uint64_t>(entry.base));
        kernel::printf(", Length=", static_cast<uint32_t>(entry.length));
        kernel::printf(", Type=", entry.type);
        kernel::printf(", ACPI_ATTRS=", entry.acpi_attrs, '\n');
    }

    kernel::printf("Kernel End="); 
    kernel::printfHex(KERNEL_END_ADDRESS);
    kernel::printf('\n');
}
