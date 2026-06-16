#pragma once

#include <type_traits>

#include "kernel/acpi/rsdp.hpp"
#include "kernel/acpi/sdt.hpp"
#include "kernel/error/error.hpp"
#include "kernel/memory/memory-defs.hpp"

#include "kernel/utils/string.hpp"

namespace kernel {
    template<typename Table_T>
    inline Table_T* findTable(const char* SIGNATURE) {
        KERNEL_ASSERT(sdtHeader != nullptr);

        bool isXsdt = rsdp.revision >= 2;

        uint32_t pointer_size = isXsdt ? 8 : 4;
        uint32_t entries = (sdtHeader->length - sizeof(SDTHeader)) / pointer_size;

        uint8_t* entry_ptr = reinterpret_cast<uint8_t*>(sdtHeader) + sizeof(SDTHeader);
        for (uint32_t i = 0; i < entries; ++i) {
            uint64_t entry_addr = isXsdt
                ? *reinterpret_cast<uint64_t*>(entry_ptr + i * 8)
                : *reinterpret_cast<uint32_t*>(entry_ptr + i * 4);

            SDTHeader* entry = reinterpret_cast<SDTHeader*>(TO_VIRT(entry_addr));
            if (kernel::equalsN(entry->signature, SIGNATURE, 4)) {
                return reinterpret_cast<Table_T*>(TO_VIRT(entry_addr));
            }
        }

        return nullptr;
    }

}
