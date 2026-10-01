#include "boot/stages/stage2.hpp"

extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "boot/memory/alloc.hpp"
#include "boot/bootInfo.hpp"

#include "common/cpuid/cpuid.hpp"
#include "common/output/output-defs.hpp"

using namespace Lexvi::Types;

namespace Lexvi::Boot::Stage2 {
    Result Run(BootInfo& bootInfo) {
        Result result{};

        if (CPUID<0>{}.vendor == Vendor::INTEL) {
            LEXVI_PRINT("INTEL\n");
        }
        else if (CPUID<0>{}.vendor == Vendor::AMD) {
            LEXVI_PRINT("AMD\n");
        }
        else {
            LEXVI_PRINT("UNKNOWN\n");
        }

        while (true) {}

        return result;
    }
}
