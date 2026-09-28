#pragma once

#include <cstdint>

#include "common/memory/address.hpp"

namespace Lexvi::Kernel {
    // Mirrors the `.header` section emitted by kernel.ld at the very start
    // of kernel.bin. Any change here MUST be mirrored there, and vice versa.
    struct KernelHeader {
        uint32_t magic;
        uint32_t version;
        uint64_t kernelSize;  // _kernel_end - KERNEL_VIRT_BASE (bytes, includes .bss)
    };
    static_assert(sizeof(KernelHeader) == 16, "kernel.ld header layout drifted");

    inline constexpr uint32_t KERNEL_HEADER_MAGIC = 0x4C455856; // 'LEXV'
    inline constexpr Memory::PhysicalAddress KERNEL_MAIN_LOAD_ADDR = 0x100000;
}

extern "C" {
    extern char stack_top[];
    extern char stack_bottom[];
    extern char _kernel_end[];
}
