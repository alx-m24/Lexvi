#include "efi/efi.h"

#include <cstdint>
#include <utility>

#include "boot/bootContext.hpp"
#include "common/memory/address.hpp"

namespace Lexvi::Boot::Memory {
    template<typename T>
    inline Lexvi::Memory::PhysicalAddress AllocPersistent() {
        constexpr UINTN PagesNeeded = EFI_SIZE_TO_PAGES(sizeof(T));

        EFI_PHYSICAL_ADDRESS address = UINT64_MAX;
        EFI_STATUS status = uefi_call_wrapper((void*)bootContext.system_table->BootServices->AllocatePages, 4,
            AllocateAnyPages,
            EfiLoaderData,
            PagesNeeded,
            &address
        );
        if (EFI_ERROR(status) || address == UINT64_MAX) {
            return Lexvi::Memory::PhysicalAddress{ nullptr };
        }

        return Lexvi::Memory::PhysicalAddress{ address };
    }

    template<typename T>
    inline Lexvi::Memory::PhysicalAddress AllocBootLifetime() {
        constexpr UINTN PagesNeeded = EFI_SIZE_TO_PAGES(sizeof(T));

        EFI_PHYSICAL_ADDRESS address = UINT64_MAX;
        EFI_STATUS status = uefi_call_wrapper((void*)bootContext.system_table->BootServices->AllocatePages, 4,
            AllocateAnyPages,
            EfiBootServicesData,
            PagesNeeded,
            &address
        );
        if (EFI_ERROR(status) || address == UINT64_MAX) {
            return Lexvi::Memory::PhysicalAddress{ nullptr };
        }

        return Lexvi::Memory::PhysicalAddress{ address };
    }

    inline Lexvi::Memory::PhysicalAddress AllocPersistent(uint64_t size) {
        const UINTN PagesNeeded = EFI_SIZE_TO_PAGES(size);

        EFI_PHYSICAL_ADDRESS address = UINT64_MAX;
        EFI_STATUS status = uefi_call_wrapper((void*)bootContext.system_table->BootServices->AllocatePages, 4,
            AllocateAnyPages,
            EfiLoaderData,
            PagesNeeded,
            &address
        );
        if (EFI_ERROR(status) || address == UINT64_MAX) {
            return Lexvi::Memory::PhysicalAddress{ nullptr };
        }

        return Lexvi::Memory::PhysicalAddress{ address };
    }

    inline Lexvi::Memory::PhysicalAddress AllocBootLifetime(uint64_t size) {
        const UINTN PagesNeeded = EFI_SIZE_TO_PAGES(size);

        EFI_PHYSICAL_ADDRESS address = UINT64_MAX;
        EFI_STATUS status = uefi_call_wrapper((void*)bootContext.system_table->BootServices->AllocatePages, 4,
            AllocateAnyPages,
            EfiBootServicesData,
            PagesNeeded,
            &address
        );
        if (EFI_ERROR(status) || address == UINT64_MAX) {
            return Lexvi::Memory::PhysicalAddress{ nullptr };
        }

        return Lexvi::Memory::PhysicalAddress{ address };
    }
    
    template<typename T, typename... Args>
    Lexvi::Memory::PhysicalAddress PersistentNew(Args&&... args) {
        Lexvi::Memory::PhysicalAddress address = AllocPersistent<T>();
    
        new (address.getValue()) T(std::forward<Args>(args)...);
    
        return address;
    }
}
