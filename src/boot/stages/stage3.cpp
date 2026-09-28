#include "boot/stages/stage3.hpp"

extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "boot/memory/alloc.hpp"
#include "boot/bootInfo.hpp"

#include "common/gdt/gdt.hpp"
#include "common/memory/pmm.hpp"
#include "common/interrupt/idt.hpp"
#include "common/types/service.hpp"

using namespace Lexvi::Memory;
using namespace Lexvi::Types;

namespace Lexvi::Boot::Stage3 {
    Result Run(BootInfo& bootInfo) {
        Result result{};

        // PMM
        PMM pmm{};
        pmm.Init();
        bootInfo.PMM_Bitmap_Address = pmm.getBitMapPhys();

        // Exit boot services

        // GDT/IDT && VMM
        using CommonPostBootServices_T = ServiceList<GDT::GDT, Interrupt::IDT /*, TODO: VMM*/>;
        CommonPostBootServices_T commonPostBootServices{};
        result = commonPostBootServices.InitAll(); 
        if (result.IsError()) {
            return result;
        }

        return result;
    }
}
