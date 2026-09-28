#include "common/memory/allocator.hpp"

extern "C" {
#include <efi/efi.h>
}

#include "common/memory/memory-unit.hpp"
#include "boot/bootContext.hpp"

namespace Lexvi::Memory {
    void* Allocate(std::size_t bytes, std::size_t alignment) {
        LEXVI_ASSERT(alignment != 0);
        LEXVI_ASSERT((alignment & (alignment - 1)) == 0);

        const std::size_t total = bytes + alignment - 1 + sizeof(void*);
    
        void* raw = nullptr;
    
        EFI_STATUS status = uefi_call_wrapper(
            (void*)bootContext.system_table->BootServices->AllocatePool,
            3,
            EfiLoaderData,
            total,
            &raw
        );
    
        LEXVI_ASSERT(!EFI_ERROR(status));
    
        Bytes address{ reinterpret_cast<uint64_t>(raw) };
    
        Bytes aligned = address.align_up(Bytes(alignment));
    
        // Store original allocation immediately before aligned address
        reinterpret_cast<void**>(aligned.count())[-1] = raw;
    
        return reinterpret_cast<void*>(aligned.count());
    }

    void Deallocate(void* ptr, std::size_t, std::size_t) noexcept {
        void* raw = reinterpret_cast<void**>(ptr)[-1];
    
        uefi_call_wrapper(
            (void*)bootContext.system_table->BootServices->FreePool,
            1,
            raw
        );
    }
}
