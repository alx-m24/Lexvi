extern "C" {
#include <efi/efi.h>
#include <efi/efilib.h>
#include "kernel/kernel-config.hpp"
}
#include "kernel/memory/memory-unit.hpp"
#include "kernel/memory/memory-manager.hpp"
#include "kernel/memory/internals/memory-map.hpp"

EFI_STATUS LoadKernelBinary(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, UINTN* OutKernelSize, UINTN* outImageBase, UINTN* outImageSize) {
    EFI_STATUS status;

    Print((const CHAR16*)u"ImageHandle=%lx BS=%lx HandleProtocolFn=%lx\n",
      (UINTN)ImageHandle,
      (UINTN)SystemTable->BootServices,
      (UINTN)SystemTable->BootServices->HandleProtocol);
    
    // 1. Locate the Loaded Image Protocol
    EFI_LOADED_IMAGE* loaded_image = nullptr;
    EFI_GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    status = uefi_call_wrapper((void*)SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &loaded_image_guid, reinterpret_cast<void**>(&loaded_image));
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"HandleProtocol ImageHandle failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        return status;
    }
    *outImageBase = reinterpret_cast<uint64_t>(loaded_image->ImageBase); 
    *outImageSize = loaded_image->ImageSize; 

    Print((const CHAR16*)u"Successfully got image handle\n");

    // 2. Locate the File System Protocol
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_system = nullptr;
    EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    status = uefi_call_wrapper((void*)SystemTable->BootServices->HandleProtocol, 3, loaded_image->DeviceHandle, &fs_guid, reinterpret_cast<void**>(&file_system));
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"HandleProtocol DeviceHandle failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        return status;
    }

    Print((const CHAR16*)u"Successfully got device handle\n");

    // 3. Open the Root Volume Directory
    EFI_FILE_PROTOCOL* root_dir = nullptr;
    status = uefi_call_wrapper((void*)file_system->OpenVolume, 2, file_system, &root_dir);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"OpenVolume failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        return status;
    }

    Print((const CHAR16*)u"Successfully opened root dir\n");

    // 4. Open "kernel.bin"
    EFI_FILE_PROTOCOL* kernel_file = nullptr;
    status = uefi_call_wrapper((void*)root_dir->Open, 5, root_dir, &kernel_file, (CHAR16*)(u"kernel.bin"), EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Open kernel_file failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        root_dir->Close(root_dir);
        return status;
    }
    
    Print((const CHAR16*)u"Successfully opened kernel.bin\n");

    // 5. Query the precise file size
    EFI_FILE_INFO* file_info = nullptr;
    UINTN info_size = 0;
    EFI_GUID file_info_guid = EFI_FILE_INFO_ID;
    
    // First call to fetch descriptor info block size needed
    uefi_call_wrapper((void*)kernel_file->GetInfo, 4, kernel_file, &file_info_guid, &info_size, nullptr);
    status = uefi_call_wrapper((void*)SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, info_size, reinterpret_cast<void**>(&file_info));
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"AllocatePool(file_info) failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    Print((const CHAR16*)u"Successfully got descriptor info\n");
    
    // Second call to grab metadata attributes
    status = uefi_call_wrapper((void*)kernel_file->GetInfo, 4, kernel_file, &file_info_guid, &info_size, file_info);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"GetInfo(fill) failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        SystemTable->BootServices->FreePool(file_info);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    UINTN kernel_size = file_info->FileSize;
    *OutKernelSize = kernel_size;
    SystemTable->BootServices->FreePool(file_info);

    Print((const CHAR16*)u"Successfully read kernel size\n");

    // 6. Allocate a safe staging pool buffer guaranteed to be valid in UEFI page tables
    void* staging_buffer = nullptr;
    status = uefi_call_wrapper((void*)SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, kernel_size, &staging_buffer);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"AllocatePool(staging_buffer) failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    // === PATCH 1: STREAM THE FILE INTO STAGING BUFFER ===
    UINTN bytes_to_read = kernel_size;
    status = uefi_call_wrapper((void*)kernel_file->Read, 3, kernel_file, &bytes_to_read, staging_buffer);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"kernel_file->Read failed: %r\n", status);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        SystemTable->BootServices->FreePool(staging_buffer);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }
    
    Print((const CHAR16*)u"Successfully read kernel to staging buffer\n");

    // 7. Allocate pages directly at your explicit custom address link boundary
    EFI_PHYSICAL_ADDRESS kernel_addr = KERNEL_MAIN_LOAD_ADDR;
    UINTN pages = (kernel_size + 4095) / 4096;
    status = uefi_call_wrapper((void*)SystemTable->BootServices->AllocatePages, 4,
        AllocateAddress, EfiLoaderData, pages, &kernel_addr);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"AllocatePages failed: %r (addr=0x%lx, pages=%d, size=%d)\n",
          status, kernel_addr, pages, kernel_size);
        Print((const CHAR16*)u"status raw = %lx\n", (UINTN)status);
        SystemTable->BootServices->FreePool(staging_buffer);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    // === PATCH 2: BLAST DATA FROM STAGING TO YOUR FIXED KERNEL LOAD DESTINATION ===
    uefi_call_wrapper((void*)SystemTable->BootServices->CopyMem, 3, reinterpret_cast<void*>(KERNEL_MAIN_LOAD_ADDR), staging_buffer, kernel_size);

    Print((const CHAR16*)u"Successfully copied kernel to 1MB mark\n");
    
    // 8. Free the staging memory and close references cleanly
    uefi_call_wrapper((void*)SystemTable->BootServices->FreePool, 1, staging_buffer);
    uefi_call_wrapper((void*)kernel_file->Close, 1, kernel_file);
    uefi_call_wrapper((void*)root_dir->Close, 1, root_dir);

    Print((const CHAR16*)u"Successfully closed all files\n");

    return EFI_SUCCESS;
}

EFI_STATUS TranslateUefiToKernelE820(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    constexpr UINTN kMaxAttempts = 5;
    EFI_STATUS status = EFI_SUCCESS;

    for (UINTN attempt = 0; attempt < kMaxAttempts; ++attempt) {
        UINTN memory_map_size = 0;
        EFI_MEMORY_DESCRIPTOR* uefi_map = nullptr;
        UINTN map_key = 0;
        UINTN descriptor_size = 0;
        UINT32 descriptor_version = 0;

        // 1. Ask UEFI for the map space size
        uefi_call_wrapper((void*)SystemTable->BootServices->GetMemoryMap, 5,
            &memory_map_size, nullptr, &map_key, &descriptor_size, &descriptor_version);

        // Pad the buffer slightly to account for the allocation change overhead
        memory_map_size += 2 * descriptor_size;
        status = uefi_call_wrapper((void*)SystemTable->BootServices->AllocatePool, 3,
            EfiLoaderData, memory_map_size, reinterpret_cast<void**>(&uefi_map));
        if (EFI_ERROR(status)) {
            Print((const CHAR16*)u"AllocatePool(uefi_map) failed: %r\n", status);
            return status;
        }

        // Fetch the absolute, clean layout matrix
        status = uefi_call_wrapper((void*)SystemTable->BootServices->GetMemoryMap, 5,
            &memory_map_size, uefi_map, &map_key, &descriptor_size, &descriptor_version);
        if (EFI_ERROR(status)) {
            Print((const CHAR16*)u"GetMemoryMap(fill) failed: %r\n", status);
            uefi_call_wrapper((void*)SystemTable->BootServices->FreePool, 1, uefi_map);
            return status;
        }

        // 2. Set pointers directly to your hardware targets
        auto* e820_dest_buffer = reinterpret_cast<kernel::E820Entry*>(MEMORY_MAP_ADDRESS);
        uint32_t translated_entry_count = 0;
        uint64_t total_uefi_descriptors = memory_map_size / descriptor_size;

        for (uint64_t i = 0; i < total_uefi_descriptors; ++i) {
            auto* uefi_desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
                reinterpret_cast<uint8_t*>(uefi_map) + (i * descriptor_size));

            e820_dest_buffer[translated_entry_count].base       = uefi_desc->PhysicalStart;
            e820_dest_buffer[translated_entry_count].length     = uefi_desc->NumberOfPages * 4096;
            e820_dest_buffer[translated_entry_count].acpi_attrs = 1;

            switch (uefi_desc->Type) {
                case EfiConventionalMemory:
                case EfiBootServicesCode:
                case EfiBootServicesData:
                    e820_dest_buffer[translated_entry_count].type = kernel::EntryType::Usable;
                    break;
                case EfiACPIReclaimMemory:
                    e820_dest_buffer[translated_entry_count].type = kernel::EntryType::ACPI_Reclaimable;
                    break;
                case EfiACPIMemoryNVS:
                    e820_dest_buffer[translated_entry_count].type = kernel::EntryType::ACPI_NVS;
                    break;
                case EfiUnusableMemory:
                    e820_dest_buffer[translated_entry_count].type = kernel::EntryType::Bad_Memory;
                    break;
                default:
                    e820_dest_buffer[translated_entry_count].type = kernel::EntryType::Reserved;
                    break;
            }

            translated_entry_count++;
        }

        auto* final_count_ptr = reinterpret_cast<uint32_t*>(MEMORY_MAP_ENTRY_COUNT_ADDRESS);
        *final_count_ptr = translated_entry_count;

        // 3. Attempt ExitBootServices immediately with the map_key we just fetched —
        // no Print/AllocatePool/other BS calls between GetMemoryMap and this.
        status = uefi_call_wrapper((void*)SystemTable->BootServices->ExitBootServices, 2,
            ImageHandle, map_key);

        if (!EFI_ERROR(status)) {
            // Boot Services are gone. Do NOT call FreePool on uefi_map or Print from here on.
            return EFI_SUCCESS;
        }

        if (status == EFI_INVALID_PARAMETER) {
            // Map changed underneath us between fetch and exit — free this attempt's
            // pool allocation and retry with a fresh map/key.
            uefi_call_wrapper((void*)SystemTable->BootServices->FreePool, 1, uefi_map);
            continue;
        }

        // Any other failure is fatal — BS is still up here, so we can still report and clean up.
        Print((const CHAR16*)u"ExitBootServices failed fatally: %r\n", status);
        uefi_call_wrapper((void*)SystemTable->BootServices->FreePool, 1, uefi_map);
        return status;
    }

    Print((const CHAR16*)u"ExitBootServices kept returning EFI_INVALID_PARAMETER after %d attempts\n",
        kMaxAttempts);
    return EFI_INVALID_PARAMETER;
}

extern "C" EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print((const CHAR16*)u"Hello from Lexvi UEFI bootloader\n");

    Print((const CHAR16*)u"Loading kernel to memory...\n");
    UINTN kernel_size = 0;
    UINTN ImageBase = 0;
    UINTN ImageSize = 0;
    EFI_STATUS status = LoadKernelBinary(ImageHandle, SystemTable, &kernel_size, &ImageBase, &ImageSize);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"CRITICAL ERROR: Failed to stream kernel.bin into RAM (%r)\n", status);
        return status;
    }

    auto kernel_entry = reinterpret_cast<void (*)()>(KERNEL_VIRT_BASE);

    Print((const CHAR16*)u"Successfully loaded kernel to memory\n");

    Print((const CHAR16*)u"Loading memory map...\n");
    status = TranslateUefiToKernelE820(ImageHandle, SystemTable);
    if (EFI_ERROR(status)) {
        // Boot Services may or may not still be available depending on which branch
        // returned this, but Print is safe in every failure path above since none of
        // them return an error after a successful ExitBootServices.
        Print((const CHAR16*)u"CRITICAL ERROR: Failed to build memory map (%r)\n", status);
        return status;
    }
    // NOTE: no Print/AllocatePool/etc. here: ExitBootServices has already succeeded
    // by the time we reach this line, so Boot Services no longer exist.

    kernel::MemoryManager memoryManager{};
    memoryManager.Init(Bytes(kernel_size), Bytes(ImageBase), Bytes(ImageSize));

    kernel_entry();

    return EFI_SUCCESS;
}
