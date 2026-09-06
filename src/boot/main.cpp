// BOOT_UEFI.cpp
extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "common/asm.hpp"

using namespace Lexvi;

extern "C" EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print((const CHAR16*)u"Hello from Lexvi UEFI bootloader - DEBUG\n");

    halt();

    return EFI_SUCCESS;
}
