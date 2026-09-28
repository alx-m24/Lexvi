#pragma once

#include "memory-unit.hpp"

namespace Lexvi::Memory {
    static constexpr Bytes HHDM_BASE        { 0xFFFF800000000000UL };
    static constexpr Bytes MMIO_BASE        { 0xFFFF810000000000UL };
    static constexpr Bytes KERNEL_PHYS_BASE { 0x100000UL };
    static constexpr Bytes KERNEL_VIRT_BASE { 0xFFFFFFFF80100000UL };
    static constexpr Bytes KERNEL_VIRT_OFFSET = KERNEL_VIRT_BASE - KERNEL_PHYS_BASE;
}

// #define TO_VIRT(addr)        (HHDM_BASE        + (uint64_t)(addr))
// #define TO_PHYS(addr)        ((uint64_t)(addr) - HHDM_BASE)
// #define KERN_TO_VIRT(addr)   (KERNEL_VIRT_OFFSET + (uint64_t)(addr))
// #define KERN_TO_PHYS(addr)   ((uint64_t)(addr)   - KERNEL_VIRT_OFFSET)
// #define MMIO_TO_PHYS(addr)   ((uint64_t)(addr) - MMIO_BASE)
// #define MMIO_TO_VIRT(addr)   (MMIO_BASE + (uint64_t)(addr))
