#include "kernel/acpi/rsdp.hpp"

#include "kernel/console/console.hpp"

RSDP rsdp {};

static bool validateRSDP(const uint8_t* ptr) {
    uint8_t sum = 0;
    for (int i = 0; i < 20; ++i) // first 20 bytes for ACPI 1.0
        sum += ptr[i];
    return sum == 0;
}

void setRSDP(RSDP* rsdpAddr) {
    rsdp = *rsdpAddr;
    kernel::printf("        - Signature: ");
    for (int i = 0; i < 8; ++i) kernel::printf(rsdp.signature[i]);
    kernel::printf('\n');

    kernel::printf("        - OEM ID: ");
    for (int i = 0; i < 6; ++i) kernel::printf(rsdp.oem_id[i]);
    kernel::printf('\n');

    kernel::printf("        - Revision: ", static_cast<uint32_t>(rsdp.revision), '\n');

    kernel::printf("        - RSDT Address: "); kernel::printfHex(rsdp.rsdt_address); kernel::printf('\n');

    if (rsdp.revision >= 2) {
        kernel::printf("        - XSDT Address: "); kernel::printfHex(rsdp.xsdt_address); kernel::printf('\n');
        kernel::printf("        - Length: ", rsdp.length, '\n');
    }
}

void rsdp_load() {
    constexpr char RSDP_SIGNATURE[] = "RSD PTR ";

    // Search EBDA first (first 1KB)
    uint16_t ebda_segment = *reinterpret_cast<uint16_t*>(0x040E);
    const uint8_t* ebda_start = reinterpret_cast<uint8_t*>(static_cast<uint64_t>(ebda_segment) << 4);
    const uint8_t* ebda_end   = ebda_start + 1024;

    // Then the BIOS area
    const uint8_t* bios_start = reinterpret_cast<uint8_t*>(0x000E0000);
    const uint8_t* bios_end   = reinterpret_cast<uint8_t*>(0x000FFFFF);

    const uint8_t* regions[2][2] = {
        { ebda_start, ebda_end },
        { bios_start, bios_end }
    };

    for (auto& region : regions) {
        for (const uint8_t* ptr = region[0]; ptr < region[1]; ptr += 16) {
            bool match = true;
            for (int i = 0; i < 8; ++i) {
                if (reinterpret_cast<const char*>(ptr)[i] != RSDP_SIGNATURE[i]) {
                    match = false;
                    break;
                }
            }
            if (match && validateRSDP(ptr)) {
                kernel::printf("        - Found RSDP at: ");
                kernel::printfHex(reinterpret_cast<uint64_t>(ptr));
                kernel::printf('\n');
                setRSDP(reinterpret_cast<RSDP*>(const_cast<uint8_t*>(ptr)));
                return;
            }
        }
    }
    kernel::printf("        - RSDP not found\n");
}
