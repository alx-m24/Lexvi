#pragma once

#include <concepts>

#include "common/memory/address.hpp"
#include "common/acpi/sdt.hpp"
#include "common/acpi/rsdp.hpp"
#include "common/utils/string.hpp"

namespace Lexvi::Boot::ACPI {
    template <typename T>
    concept Table_T = requires {
        { T::getSignature() } -> std::same_as<const char*>;
    };

    template<Table_T T>
    inline Lexvi::Memory::PhysicalAddress findTable() {
        bool isXsdt = Lexvi::ACPI::rsdp.revision >= 2;
        uint32_t pointer_size = isXsdt ? 8 : 4;
        uint32_t entries = (Lexvi::ACPI::sdtHeader->length - sizeof(Lexvi::ACPI::SDTHeader)) / pointer_size;

        uint8_t* entry_ptr = reinterpret_cast<uint8_t*>(Lexvi::ACPI::sdtHeader) + sizeof(Lexvi::ACPI::SDTHeader);
        for (uint32_t i = 0; i < entries; ++i) {
            uint64_t entry_addr = isXsdt
                ? *reinterpret_cast<uint64_t*>(entry_ptr + i * 8)
                : *reinterpret_cast<uint32_t*>(entry_ptr + i * 4);

            // identity-mapped under UEFI — dereferenceable directly, no TO_VIRT
            Lexvi::ACPI::SDTHeader* entry = reinterpret_cast<Lexvi::ACPI::SDTHeader*>(entry_addr);
            if (equalsN(entry->signature, T::getSignature(), 4)) {
                return Lexvi::Memory::PhysicalAddress(entry_addr);
            }
        }

        return Lexvi::Memory::PhysicalAddress(nullptr);
    }
}
