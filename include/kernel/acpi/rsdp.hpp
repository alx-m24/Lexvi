#pragma once

#include "stdint.h"

struct RSDP {
    char     signature[8];  // "RSD PTR " (note the trailing space)
    uint8_t  checksum;      // sum of all bytes in v1 struct must = 0
    char     oem_id[6];
    uint8_t  revision;      // 0 = ACPI 1.0, 2 = ACPI 2.0+
    uint32_t rsdt_address;  // physical address of RSDT (32-bit, ACPI 1.0)
    // if revision >= 2:
    uint32_t length;
    uint64_t xsdt_address;  // physical address of XSDT (64-bit, ACPI 2.0+)
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} __attribute__((packed));

extern RSDP rsdp;

void rsdp_load();
