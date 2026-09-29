#include "boot/stages/stage3.hpp"

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

namespace Lexvi::Boot::Stage3 {
    static Result ExitBootServices(PMM& pmm) {
        constexpr UINTN kMaxAttempts = 5;
        EFI_STATUS status = EFI_SUCCESS;

        UINTN map_key{};
        EFI_MEMORY_DESCRIPTOR* uefi_map = nullptr;

        for (UINTN attempt = 0; attempt < kMaxAttempts; ++attempt) {
            pmm.SyncBitMap();
            pmm.getUEFIMap(map_key, uefi_map);

            // Attempt ExitBootServices immediately with the map_key
            // no Print/AllocatePool/other BS calls between GetMemoryMap and this.
            EFI_STATUS status = uefi_call_wrapper((void*)bootContext.system_table->BootServices->ExitBootServices, 2,
            bootContext.image, map_key);

            if (!EFI_ERROR(status)) {
                return { }; // Success
            }

            if (status == EFI_INVALID_PARAMETER) {
                // Map changed underneath us between fetch and exit — free this attempt's
                // pool allocation and retry with a fresh map/key.
                uefi_call_wrapper((void*)bootContext.system_table->BootServices->FreePool, 1, uefi_map);
                continue;
            }

            // Any other failure is fatal — BS is still up here, so we can still report and clean up.
            LEXVI_PRINT("ExitBootServices failed fatally: %r\n", status);
            uefi_call_wrapper((void*)bootContext.system_table->BootServices->FreePool, 1, uefi_map);
        }

        return { cstring_view("ExitBootServices kept returning EFI_INVALID_PARAMETER after multiple attempts") };
    }

    Result Run(BootInfo& bootInfo) {
        Result result{};

        // PMM
        PMM pmm{};
        pmm.Init();
        bootInfo.PMM_Bitmap_Address = pmm.getBitMapPhys();

        // Exit boot services
        ExitBootServices(pmm);

        // GDT, IDT & VMM
        using CommonPostBootServices_T = ServiceList<GDT::GDT, Interrupt::IDT, Lexvi::Memory::VMM>;
        CommonPostBootServices_T commonPostBootServices{};
        result = commonPostBootServices.InitAll(); 
        if (result.IsError()) {
            return result;
        }
        uint64_t cr3{};
        ASM("mov %%cr3, %0" : "=r"(cr3));
        bootInfo.PML4_Address = reinterpret_cast<Lexvi::Memory::PageTable*>(cr3);


        return result;
    }
}
