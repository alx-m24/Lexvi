#pragma once

#include "cstdint"

#include "kernel/debug/gop.hpp"

constexpr uint64_t KERNEL_MAIN_LOAD_ADDR = 0x100000;

constexpr uint64_t MEMORY_MAP_ADDRESS = 0x7000;
constexpr uint64_t MEMORY_MAP_ENTRY_COUNT_ADDRESS = MEMORY_MAP_ADDRESS - sizeof(uint64_t);

constexpr uint64_t PMM_BITMAP_PHYS_ADDRESS = MEMORY_MAP_ENTRY_COUNT_ADDRESS - sizeof(uint64_t);

constexpr uint64_t RSDP_ADDRESS_PHYS_ADDRESS = PMM_BITMAP_PHYS_ADDRESS - sizeof(uint64_t);

constexpr uint64_t GOP_PHYS_ADDRESS = RSDP_ADDRESS_PHYS_ADDRESS - sizeof(kernel::GOP);
constexpr uint64_t GOP_INFO_PHYS_ADDRESS = GOP_PHYS_ADDRESS - sizeof(kernel::GOP_Info);

namespace kernel {
    // Mirrors the `.header` section emitted by kernel.ld at the very start
    // of kernel.bin. Any change here MUST be mirrored there, and vice versa.
    struct KernelHeader {
        uint32_t magic;
        uint32_t version;
        uint64_t kernelSize;  // _kernel_end - KERNEL_VIRT_BASE (bytes, includes .bss)
    };
    static_assert(sizeof(KernelHeader) == 16, "kernel.ld header layout drifted");

    constexpr uint32_t KERNEL_HEADER_MAGIC = 0x4C455856; // 'LEXV'
}

extern "C" {
    extern char stack_top[];
    extern char stack_bottom[];
    extern char _kernel_end[];
}

inline uint64_t GetKernelStackSize() {
    return reinterpret_cast<uint64_t>(stack_top) - reinterpret_cast<uint64_t>(stack_bottom);
}
