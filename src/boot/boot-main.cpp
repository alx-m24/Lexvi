// BOOT_UEFI.cpp
extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "common/asm.hpp"
#include "common/memory/memory-defs.hpp"
#include "common/output/output-defs.hpp"

#include "kernel/header.hpp"

#include "boot/bootContext.hpp"
#include "boot/memory/alloc.hpp"
#include "boot/stages/stage1.hpp"
#include "boot/stages/stage3.hpp"

using namespace Lexvi;
using namespace Lexvi::Boot;

#define LEXVI_BOOT_SUCCESS() do {\
        halt(); \
        return EFI_SUCCESS; \
    } while (false)\

#define LEXVI_BOOT_FAILURE() do {\
        halt(); \
        return 1; \
    } while (false)\

extern "C" EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    LEXVI_PRINT("Hello from Lexvi UEFI bootloader - DEBUG\n");

    bootContext.image = ImageHandle;
    bootContext.system_table = SystemTable;

    BootInfo* bootInfo = reinterpret_cast<BootInfo*>(Lexvi::Boot::Memory::AllocPersistent<BootInfo>().getValue());
    *bootInfo = BootInfo{};

    Types::Result result{};
    
    // Stage 1 -> early init
    result = Boot::Stage1::Run(*bootInfo);
    if (result.IsError()) {
        LEXVI_PRINT("Failed to init Lexvi (stage 1): ", result.getError(), '\n');
        LEXVI_BOOT_FAILURE();
    }

    // Stage 2 -> load kernel

    // Stage 3 -> Exit boot service & post boot init
    result = Boot::Stage3::Run(*bootInfo);
    if (result.IsError()) {
        LEXVI_PRINT("Failed to init Lexvi (stage 3): ", result.getError(), '\n');
        LEXVI_BOOT_FAILURE();
    }

    // switch to kernel
    using KernelEntry = void (*)(BootInfo*);
    auto kernel_entry 
        = reinterpret_cast<KernelEntry>(Lexvi::Memory::KERNEL_VIRT_BASE.bytes().count() 
                + sizeof(Lexvi::Kernel::KernelHeader));
    
    kernel_entry(bootInfo);


    LEXVI_BOOT_SUCCESS();
}
