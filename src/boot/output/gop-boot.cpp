#include "common/output/gop.hpp"

extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "common/output/output-defs.hpp"
#include "common/error/panic.hpp"
#include "boot/bootContext.hpp"

namespace Lexvi::Output {
    GOP_Data gop{};

    void* get_uefi_gop(EFI_SYSTEM_TABLE *SystemTable) {
        void* gop;
        EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_STATUS status = 
            uefi_call_wrapper((void*)SystemTable->BootServices->LocateProtocol, 3, &gop_guid, nullptr, &gop); 

        if (EFI_ERROR(status)) {
            LEXVI_PANIC("Failed to locate GOP");
            return nullptr;
        }
        return gop;
    }

    Types::Result GOP::Init() {
        LEXVI_PRINT("Getting GOP\n");
        EFI_GRAPHICS_OUTPUT_PROTOCOL* gop_ptr = reinterpret_cast<EFI_GRAPHICS_OUTPUT_PROTOCOL*>(get_uefi_gop(bootContext.system_table));
        if (!gop_ptr) {
            return Types::Result{ Types::cstring_view("GOP PHYSICAL ADDRESS is NULL") };
        }

        LEXVI_PRINT(" - GOP protocol located\n");

        gop = *reinterpret_cast<GOP_Data*>(gop_ptr->Mode);

        LEXVI_PRINT(" - Successfully gotten GOP\n");

        return {};
    }

    void GOP::Shutdown() {

    }
}
