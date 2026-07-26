#pragma once

#include <cstdint>
#include "kernel/acpi/sdt.hpp"

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
} __attribute__((packed));

extern uint64_t mcfg_base;
extern uint64_t mcfg_count;

void mcfg_load();
