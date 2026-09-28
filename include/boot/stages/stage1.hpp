#pragma once

extern "C" {
    #include <efi/efi.h>
}

#include "common/types/result.hpp"
#include "boot/bootInfo.hpp"

namespace Lexvi::Boot::Stage1 {
    Types::Result Run(BootInfo& bootInfo);
}
