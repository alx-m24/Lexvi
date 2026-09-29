#pragma once

#include <cstdint>

#include "kernel/header.hpp"

namespace Lexvi::Memory {
    enum class EntryType : uint32_t {
        Usable = 1,
        Reserved = 2,
        ACPI_Reclaimable = 3,
        ACPI_NVS = 4,
        Bad_Memory = 5
    };

    struct E820Entry {
        uint64_t base;
        uint64_t length;
        EntryType type;
        uint32_t acpi_attrs;
    } __attribute__((packed));

    inline uint64_t GetKernelEndAddress() {
        return reinterpret_cast<uint64_t>(_kernel_end);
    }
};

#define KERNEL_END_ADDRESS kernel::GetKernelEndAddress()
