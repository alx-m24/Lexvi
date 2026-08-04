#include "kernel/acpi/rsdp.hpp"

#include "kernel/error/error.hpp"
#include "kernel/debug/serial.hpp"
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
    kernel::serial::put("        - Signature: ");
    for (int i = 0; i < 8; ++i) kernel::serial::put(rsdp.signature[i]);
    kernel::serial::put('\n');

    kernel::serial::put("        - OEM ID: ");
    for (int i = 0; i < 6; ++i) kernel::serial::put(rsdp.oem_id[i]);
    kernel::serial::put('\n');

    kernel::serial::put("        - Revision: ", static_cast<uint32_t>(rsdp.revision), '\n');

    kernel::serial::put("        - RSDT Address: "); kernel::serial::putHex(rsdp.rsdt_address); kernel::serial::put('\n');

    if (rsdp.revision >= 2) {
        kernel::serial::put("        - XSDT Address: "); kernel::serial::putHex(rsdp.xsdt_address); kernel::serial::put('\n');
        kernel::serial::put("        - Length: ", rsdp.length, '\n');
    }
}

void rsdp_load() {
    RSDP** rsdpAddress_ptr = reinterpret_cast<RSDP**>(TO_VIRT(RSDP_ADDRESS_PHYS_ADDRESS));
    KERNEL_ASSERT(rsdpAddress_ptr != nullptr);
    RSDP* rsdp_ptr = reinterpret_cast<RSDP*>(TO_VIRT(*rsdpAddress_ptr));
    KERNEL_ASSERT(rsdp_ptr != nullptr);
    setRSDP(rsdp_ptr);
}
