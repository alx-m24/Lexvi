#pragma once

extern "C" {
    #include <efi/efi.h>
}

#include "common/memory/memory-unit.hpp"
#include "common/memory/memory-map.hpp"

namespace Lexvi::Memory {
    struct PMM;
}

namespace Lexvi::Boot {
    struct BootContext {
        EFI_HANDLE image{};
        EFI_SYSTEM_TABLE* system_table{};

        Memory::PMM* pmm{};
        Memory::Bytes kernelSize{};

        uint64_t MEMORY_MAP_ENTRY_COUNT{};
        Lexvi::Memory::E820Entry* E820Entries{};
    };
}

extern Lexvi::Boot::BootContext bootContext;
