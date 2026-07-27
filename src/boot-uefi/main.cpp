extern "C" {
#include <efi/efi.h>
#include <efi/efilib.h>
#include "kernel/kernel-config.hpp"
}
#include "kernel/memory/internals/memory-map.hpp"

EFI_STATUS LoadKernelBinary(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, UINTN* OutKernelSize) {
    EFI_STATUS status;
    
    // 1. Locate the Loaded Image Protocol
    EFI_LOADED_IMAGE* loaded_image = nullptr;
    EFI_GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    status = SystemTable->BootServices->HandleProtocol(ImageHandle, &loaded_image_guid, reinterpret_cast<void**>(&loaded_image));
    if (EFI_ERROR(status)) return status;

    // 2. Locate the File System Protocol
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_system = nullptr;
    EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    status = SystemTable->BootServices->HandleProtocol(loaded_image->DeviceHandle, &fs_guid, reinterpret_cast<void**>(&file_system));
    if (EFI_ERROR(status)) return status;

    // 3. Open the Root Volume Directory
    EFI_FILE_PROTOCOL* root_dir = nullptr;
    status = file_system->OpenVolume(file_system, &root_dir);
    if (EFI_ERROR(status)) return status;

    // 4. Open "kernel.bin"
    EFI_FILE_PROTOCOL* kernel_file = nullptr;
    status = root_dir->Open(root_dir, &kernel_file, (CHAR16*)(u"kernel.bin"), EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        root_dir->Close(root_dir);
        return status;
    }

    // 5. Query the precise file size
    EFI_FILE_INFO* file_info = nullptr;
    UINTN info_size = 0;
    EFI_GUID file_info_guid = EFI_FILE_INFO_ID;
    
    // First call to fetch descriptor info block size needed
    kernel_file->GetInfo(kernel_file, &file_info_guid, &info_size, nullptr);
    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, info_size, reinterpret_cast<void**>(&file_info));
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"AllocatePool(file_info) failed: %r\n", status);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }
    
    // Second call to grab metadata attributes
    status = kernel_file->GetInfo(kernel_file, &file_info_guid, &info_size, file_info);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"GetInfo(fill) failed: %r\n", status);
        SystemTable->BootServices->FreePool(file_info);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    UINTN kernel_size = file_info->FileSize;
    *OutKernelSize = kernel_size;
    SystemTable->BootServices->FreePool(file_info);

    // 6. Allocate a safe staging pool buffer guaranteed to be valid in UEFI page tables
    void* staging_buffer = nullptr;
    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, kernel_size, &staging_buffer);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"AllocatePool(staging_buffer) failed: %r\n", status);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    // === PATCH 1: STREAM THE FILE INTO STAGING BUFFER ===
    UINTN bytes_to_read = kernel_size;
    status = kernel_file->Read(kernel_file, &bytes_to_read, staging_buffer);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"kernel_file->Read failed: %r\n", status);
        SystemTable->BootServices->FreePool(staging_buffer);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    // 7. Allocate pages directly at your explicit custom address link boundary
    EFI_PHYSICAL_ADDRESS kernel_addr = KERNEL_MAIN_LOAD_ADDR;
    UINTN pages = (kernel_size + 4095) / 4096;
    status = SystemTable->BootServices->AllocatePages(
        AllocateAddress, EfiLoaderData, pages, &kernel_addr);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"AllocatePages failed: %r (addr=0x%lx, pages=%d, size=%d)\n",
          status, kernel_addr, pages, kernel_size);
        SystemTable->BootServices->FreePool(staging_buffer);
        kernel_file->Close(kernel_file);
        root_dir->Close(root_dir);
        return status;
    }

    // === PATCH 2: BLAST DATA FROM STAGING TO YOUR FIXED KERNEL LOAD DESTINATION ===
    SystemTable->BootServices->CopyMem(reinterpret_cast<void*>(KERNEL_MAIN_LOAD_ADDR), staging_buffer, kernel_size);

    // 8. Free the staging memory and close references cleanly
    SystemTable->BootServices->FreePool(staging_buffer);
    kernel_file->Close(kernel_file);
    root_dir->Close(root_dir);

    return EFI_SUCCESS;
}


void TranslateUefiToKernelE820(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    UINTN memory_map_size = 0;
    EFI_MEMORY_DESCRIPTOR* uefi_map = nullptr;
    UINTN map_key = 0;
    UINTN descriptor_size = 0;
    UINT32 descriptor_version = 0;

    // 1. Ask UEFI for the map space size
    SystemTable->BootServices->GetMemoryMap(&memory_map_size, nullptr, &map_key, &descriptor_size, &descriptor_version);
    
    // Pad the buffer slightly to account for the allocation change overhead
    memory_map_size += 2 * descriptor_size; 
    SystemTable->BootServices->AllocatePool(EfiLoaderData, memory_map_size, reinterpret_cast<void**>(&uefi_map));
    
    // Fetch the absolute, clean layout matrix
    SystemTable->BootServices->GetMemoryMap(&memory_map_size, uefi_map, &map_key, &descriptor_size, &descriptor_version);

    // 2. Set pointers directly to your hardware targets
    auto* e820_dest_buffer = reinterpret_cast<kernel::E820Entry*>(MEMORY_MAP_ADDRESS);
    uint32_t translated_entry_count = 0;
    uint64_t total_uefi_descriptors = memory_map_size / descriptor_size;

    for (uint64_t i = 0; i < total_uefi_descriptors; ++i) {
        auto* uefi_desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(reinterpret_cast<uint8_t*>(uefi_map) + (i * descriptor_size));

        e820_dest_buffer[translated_entry_count].base       = uefi_desc->PhysicalStart;
        e820_dest_buffer[translated_entry_count].length     = uefi_desc->NumberOfPages * 4096;
        e820_dest_buffer[translated_entry_count].acpi_attrs = 1; // Standard extended mapping validation flag

        // 3. Map modern firmware descriptors to your explicit EntryType tracking layout
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
                // Everything else (Runtime services, MMIO, Reserved) is flagged safely out of range
                e820_dest_buffer[translated_entry_count].type = kernel::EntryType::Reserved;
                break;
        }
        
        translated_entry_count++;
    }

    // 4. Commit the literal translated entry metric directly to your pointer targets
    auto* final_count_ptr = reinterpret_cast<uint32_t*>(MEMORY_MAP_ENTRY_COUNT_ADDRESS);
    *final_count_ptr = translated_entry_count;

    EFI_STATUS status = SystemTable->BootServices->ExitBootServices(ImageHandle, map_key);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to ExitBootServices\n");
        return; 
    }
}

extern "C" EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print((const CHAR16*)u"Hello from Lexvi UEFI bootloader\n");

    Print((const CHAR16*)u"Loading kernel to memory...\n");
    UINTN kernel_size = 0;
    EFI_STATUS status = LoadKernelBinary(ImageHandle, SystemTable, &kernel_size);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"CRITICAL ERROR: Failed to stream kernel.bin into RAM (%r)\n", status);
        return status;
    }

    auto kernel_entry = reinterpret_cast<void (*)()>(KERNEL_MAIN_LOAD_ADDR);

    Print((const CHAR16*)u"Successfully loaded kernel to memory\n");

    Print((const CHAR16*)u"Loading memory map...\n");
    TranslateUefiToKernelE820(ImageHandle, SystemTable);
    
    kernel_entry();

    return EFI_SUCCESS;
}
