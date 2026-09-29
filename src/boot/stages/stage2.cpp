#include "boot/stages/stage2.hpp"

extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "boot/memory/alloc.hpp"
#include "boot/bootInfo.hpp"

#include "common/asm.hpp"

#include "common/gdt/gdt.hpp"
#include "common/memory/pmm.hpp"
#include "common/memory/vmm.hpp"
#include "common/interrupt/idt.hpp"
#include "common/types/service.hpp"
#include "common/output/output-defs.hpp"

using namespace Lexvi::Memory;
using namespace Lexvi::Types;

namespace Lexvi::Boot::Stage2 {
    Result Run(BootInfo& bootInfo) {
        Result result{};

        return result;
    }
}
