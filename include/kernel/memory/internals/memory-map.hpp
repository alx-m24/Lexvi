#pragma once

#include <cstdint>

#include "kernel/kernel-config.hpp"

namespace kernel {
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

    inline uint32_t GetMemoryMapEntryCount() {
        return *reinterpret_cast<uint32_t*>(MEMORY_MAP_ENTRY_COUNT_ADDRESS);
    }
    inline const E820Entry* GetE820Entries() {
        return reinterpret_cast<const E820Entry*>(MEMORY_MAP_ADDRESS);
    }
};

#define MEMORY_MAP_ENTRY_COUNT kernel::GetMemoryMapEntryCount()
#define E820Entries kernel::GetE820Entries()
#define KERNEL_END_ADDRESS kernel::GetKernelEndAddress()
