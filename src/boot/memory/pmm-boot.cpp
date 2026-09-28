#include "common/memory/pmm.hpp"

extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "common/utils/math.hpp"
#include "boot/bootContext.hpp"
#include "boot/memory/alloc.hpp"

#include <cstdint>

using namespace Lexvi::Types;
using namespace Lexvi::Boot;
using namespace Lexvi::Memory;
using namespace Lexvi::Utils::Math;

namespace Lexvi::Memory {
    constexpr uint64_t CalculateBitmapSize(uint64_t physicalLimit) {
        constexpr uint64_t PageSize = PMM::PAGE_SIZE.bytes().count();
        constexpr uint64_t BitsPerByte = 8;
    
        uint64_t pageCount = (physicalLimit + PageSize - 1) / PageSize;
    
        return (pageCount + BitsPerByte - 1) / BitsPerByte;
    }

    static Result getMemoryMap(BootContext& bootContext, EFI_MEMORY_DESCRIPTOR*& out_memoryMap, uint64_t& out_EntryCount, uint64_t& out_DescriptorSize) {
        UINTN memory_map_size = 0;
        EFI_MEMORY_DESCRIPTOR* uefi_map = nullptr;
        UINTN map_key = 0;
        UINTN descriptor_size = 0;
        UINT32 descriptor_version = 0;

        // calculate size
        memory_map_size = 0;
        uefi_call_wrapper((void*)bootContext.system_table->BootServices->GetMemoryMap, 5,
                &memory_map_size, nullptr, &map_key, &descriptor_size, &descriptor_version);

        // Pad the buffer slightly to account for the allocation change overhead
        memory_map_size += 2 * descriptor_size;
        uefi_map = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(Boot::Memory::AllocBootLifetime(memory_map_size).getValue());
        if (uefi_map == nullptr) {
            return Result{ cstring_view("AllocatePool(uefi_map) failed") };
        }

        // Fetch the absolute, clean layout matrix
        EFI_STATUS status = uefi_call_wrapper((void*)bootContext.system_table->BootServices->GetMemoryMap, 5,
            &memory_map_size, uefi_map, &map_key, &descriptor_size, &descriptor_version);
        if (EFI_ERROR(status)) {
            uefi_call_wrapper((void*)bootContext.system_table->BootServices->FreePool, 1, uefi_map);
            return Result{ cstring_view("GetMemoryMap(fill) failed") };
        }

        const uint64_t total_uefi_descriptors = memory_map_size / descriptor_size;

        out_memoryMap = uefi_map;
        out_EntryCount = total_uefi_descriptors;
        out_DescriptorSize = descriptor_size;

        return Result{};
    }

    static Result getHighestAddress(BootContext& bootContext, uint64_t& out_highestAddress) {
        EFI_MEMORY_DESCRIPTOR* uefi_map = nullptr;
        uint64_t total_uefi_descriptors = 0;
        UINTN descriptor_size = 0;

        getMemoryMap(bootContext, uefi_map, total_uefi_descriptors, descriptor_size);

        out_highestAddress = 0;

        for (uint64_t i = 0; i < total_uefi_descriptors; ++i) {
            auto* uefi_desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
                reinterpret_cast<uint8_t*>(uefi_map) + (i * descriptor_size));

            uint64_t end = uefi_desc->PhysicalStart + uefi_desc->NumberOfPages * EFI_PAGE_SIZE;

            out_highestAddress = max(out_highestAddress, end);
        }

        uefi_call_wrapper((void*)bootContext.system_table->BootServices->FreePool, 1, uefi_map);
        return {};
    }

    Result PMM::Init() {
        Types::Result result{};

        bootContext.pmm = this;

        uint64_t maxAddressToCover{};
        result = getHighestAddress(bootContext, maxAddressToCover);
        if (result.IsError()) {
            return result;
        }

        const uint64_t bitMapSize = CalculateBitmapSize(maxAddressToCover);
        EFI_PHYSICAL_ADDRESS bitmapAddress = Boot::Memory::AllocPersistent(bitMapSize).getValue();
        if (bitmapAddress == 0) {
            return Result{ cstring_view("AllocatePages(PMM bitmap) failed") };
        }

        m_bitMap = PhysicalAddress(Bytes(bitmapAddress));
        m_TotalPageNum = (maxAddressToCover + PAGE_SIZE.bytes().count() - 1) / PAGE_SIZE.bytes().count();
        m_bitMapSize = bitMapSize;

        SyncBitMap();

        return result;
    }

    Result PMM::SyncBitMap() {
        EFI_MEMORY_DESCRIPTOR* uefi_map = nullptr;
        uint64_t total_uefi_descriptors = 0;
        UINTN descriptor_size = 0;
    
        const Result result = getMemoryMap(
            bootContext,
            uefi_map,
            total_uefi_descriptors,
            descriptor_size
        );
    
        if (result.IsError()) {
            return result;
        }
    
        for (uint64_t page = 0; page < m_TotalPageNum; ++page) {
            SetBit(page);
        }
    
        for (uint64_t i = 0; i < total_uefi_descriptors; ++i) {
            auto* uefi_desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(
                reinterpret_cast<uint8_t*>(uefi_map) +
                (i * descriptor_size)
            );
    
            const Bytes base{ uefi_desc->PhysicalStart };
            const Bytes length{
                uefi_desc->NumberOfPages * EFI_PAGE_SIZE
            };
    
            switch (uefi_desc->Type) {
                // Not used anymore after ExitBootServices
                case EfiConventionalMemory:
                case EfiACPIReclaimMemory:
                case EfiBootServicesCode:
                case EfiBootServicesData:
                    MarkRangeFree(base, length);
                    break;
    
                // kernel/bootloader may still have code,
                case EfiLoaderCode:
                case EfiLoaderData:
    
                // Must remain reserved.
                case EfiRuntimeServicesCode:
                case EfiRuntimeServicesData:
                case EfiACPIMemoryNVS:
                case EfiMemoryMappedIO:
                case EfiMemoryMappedIOPortSpace:
                case EfiPalCode:
                case EfiUnusableMemory:
                case EfiReservedMemoryType:
                default:
                    break;
            }
        }
    
        uefi_call_wrapper((void*)bootContext.system_table->BootServices->FreePool, 1, uefi_map);

        return {};
    }

    void PMM::Shutdown() {

    }
}
