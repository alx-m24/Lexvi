#pragma once

#include <cstdint>
#include "common/acpi/sdt.hpp"

namespace Lexvi::ACPI {
struct MCFGEntry {
    uint64_t base_address;
    uint16_t segment_group;
    uint8_t  start_bus;
    uint8_t  end_bus;
    uint32_t reserved;
} __attribute__((packed));

struct MCFGTable {
    SDTHeader header;
    uint64_t  reserved;

    static constexpr const char* getSignature() {
        return "MCFG";
    }
} __attribute__((packed));
}
