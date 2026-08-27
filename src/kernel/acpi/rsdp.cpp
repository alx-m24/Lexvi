#include "kernel/acpi/rsdp.hpp"

#include "kernel/error/error.hpp"
#include "kernel/debug/gop.hpp"
#include "kernel/kernel-config.hpp"
#include "kernel/memory/memory-defs.hpp"

RSDP rsdp {};

static bool validateRSDP(const uint8_t* ptr) {
    uint8_t sum = 0;
    for (int i = 0; i < 20; ++i) // first 20 bytes for ACPI 1.0
        sum += ptr[i];
    return sum == 0;
}

void setRSDP(RSDP* rsdpAddr) {
    rsdp = *rsdpAddr;
    KERNEL_PRINT("        - Signature: ");
    for (int i = 0; i < 8; ++i) KERNEL_PRINT(rsdp.signature[i]);
    KERNEL_PRINT('\n');

    KERNEL_PRINT("        - OEM ID: ");
    for (int i = 0; i < 6; ++i) KERNEL_PRINT(rsdp.oem_id[i]);
    KERNEL_PRINT('\n');

    KERNEL_PRINT("        - Revision: ", static_cast<uint32_t>(rsdp.revision), '\n');

    KERNEL_PRINT("        - RSDT Address: "); KERNEL_PRINTHEX(rsdp.rsdt_address); KERNEL_PRINT('\n');

    if (rsdp.revision >= 2) {
        KERNEL_PRINT("        - XSDT Address: "); KERNEL_PRINTHEX(rsdp.xsdt_address); KERNEL_PRINT('\n');
        KERNEL_PRINT("        - Length: ", rsdp.length, '\n');
    }
}

void rsdp_load() {
    RSDP** rsdpAddress_ptr = reinterpret_cast<RSDP**>(TO_VIRT(RSDP_ADDRESS_PHYS_ADDRESS));
    KERNEL_ASSERT(rsdpAddress_ptr != nullptr);
    RSDP* rsdp_ptr = reinterpret_cast<RSDP*>(TO_VIRT(*rsdpAddress_ptr));
    KERNEL_ASSERT(rsdp_ptr != nullptr);
    setRSDP(rsdp_ptr);
}
