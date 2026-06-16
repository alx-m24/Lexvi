#pragma once
#include <cstdint>
#include "memory-unit.hpp"

static constexpr uint64_t HHDM_BASE         = 0xFFFF800000000000ULL;
static constexpr uint64_t MMIO_BASE         = 0xFFFF810000000000ULL;
static constexpr uint64_t KERNEL_PHYS_BASE  = 0x100000ULL;
static constexpr uint64_t KERNEL_VIRT_BASE  = 0xFFFFFFFF80100000ULL;
static constexpr uint64_t KERNEL_VIRT_OFFSET = KERNEL_VIRT_BASE - KERNEL_PHYS_BASE;

static constexpr KiB PAGE_SIZE = 4_KiB;

#ifdef BOOTLOADER
    #define TO_VIRT(addr)        ((uint64_t)(addr))
    #define TO_PHYS(addr)        ((uint64_t)(addr))
    #define KERN_TO_VIRT(addr)   ((uint64_t)(addr))
    #define KERN_TO_PHYS(addr)   ((uint64_t)(addr))
    #define MMIO_TO_PHYS(addr)   ((uint64_t)(addr))
    #define MMIO_TO_VIRT(addr)   ((uint64_t)(addr))
#else
    #define TO_VIRT(addr)        (HHDM_BASE        + (uint64_t)(addr))
    #define TO_PHYS(addr)        ((uint64_t)(addr) - HHDM_BASE)
    #define KERN_TO_VIRT(addr)   (KERNEL_VIRT_OFFSET + (uint64_t)(addr))
    #define KERN_TO_PHYS(addr)   ((uint64_t)(addr)   - KERNEL_VIRT_OFFSET)
    #define MMIO_TO_PHYS(addr)   ((uint64_t)(addr) - MMIO_BASE)
    #define MMIO_TO_VIRT(addr)   (MMIO_BASE + (uint64_t)(addr))
#endif
