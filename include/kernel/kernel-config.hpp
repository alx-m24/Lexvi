#pragma once

#include <cstdint>

inline constexpr uint64_t KERNEL_MAIN_LOAD_ADDR = 0x100000;

extern uint64_t MEMORY_MAP_ADDRESS;

uint64_t get_MEMORY_MAP_ENTRY_COUNT_ADDRESS();

uint64_t get_PMM_BITMAP_PHYS_ADDRESS();

uint64_t get_RSDP_ADDRESS_PHYS_ADDRESS();

uint64_t get_GOP_PHYS_ADDRESS();

uint64_t get_GOP_INFO_PHYS_ADDRESS();

uint64_t get_LOWEST_ADDRESS();

#define MEMORY_MAP_ENTRY_COUNT_ADDRESS  get_MEMORY_MAP_ENTRY_COUNT_ADDRESS()
#define PMM_BITMAP_PHYS_ADDRESS         get_PMM_BITMAP_PHYS_ADDRESS()
#define RSDP_ADDRESS_PHYS_ADDRESS       get_RSDP_ADDRESS_PHYS_ADDRESS()
#define GOP_PHYS_ADDRESS                get_GOP_PHYS_ADDRESS()
#define GOP_INFO_PHYS_ADDRESS           get_GOP_INFO_PHYS_ADDRESS()

inline constexpr uint64_t TOTAL_ADDRESSES_SIZE = 
    sizeof(MEMORY_MAP_ADDRESS)
    + sizeof(MEMORY_MAP_ENTRY_COUNT_ADDRESS)
    + sizeof(PMM_BITMAP_PHYS_ADDRESS)
    + sizeof(RSDP_ADDRESS_PHYS_ADDRESS)
    + sizeof(GOP_PHYS_ADDRESS)
    + sizeof(GOP_INFO_PHYS_ADDRESS);

namespace kernel {
    // Mirrors the `.header` section emitted by kernel.ld at the very start
    // of kernel.bin. Any change here MUST be mirrored there, and vice versa.
    struct KernelHeader {
        uint32_t magic;
        uint32_t version;
        uint64_t kernelSize;  // _kernel_end - KERNEL_VIRT_BASE (bytes, includes .bss)
    };
    static_assert(sizeof(KernelHeader) == 16, "kernel.ld header layout drifted");

    inline constexpr uint32_t KERNEL_HEADER_MAGIC = 0x4C455856; // 'LEXV'
}

extern "C" {
    extern char stack_top[];
    extern char stack_bottom[];
    extern char _kernel_end[];
}

uint64_t GetKernelStackSize();
