#pragma once

#include "common/memory/address.hpp"

namespace Lexvi::Boot {
    struct BootInfo {
        Lexvi::Memory::PhysicalAddress PMM_Bitmap_Address{};
        Lexvi::Memory::PhysicalAddress RSDP_Address{};
        Lexvi::Memory::PhysicalAddress SDTHeader_Address{};
        Lexvi::Memory::PhysicalAddress FADT_Address{};
        Lexvi::Memory::PhysicalAddress HPET_Address{};
        Lexvi::Memory::PhysicalAddress MCFG_Address{};
        Lexvi::Memory::PhysicalAddress GOP_Address{};
    };
}
