#pragma once

extern "C" {
    #include <efi/efi.h>
}

#include "common/memory/memory-unit.hpp"

namespace Lexvi::Memory {
    struct PMM;
}

namespace Lexvi::Boot {
    struct BootContext {
        EFI_HANDLE image{};
        EFI_SYSTEM_TABLE* system_table{};

        Memory::PMM* pmm{};
    };
}

extern Lexvi::Boot::BootContext bootContext;
