#include "kernel/kernel-config.hpp"

#include "kernel/utils/math.hpp"
#include "kernel/debug/gop.hpp"

uint64_t MEMORY_MAP_ADDRESS{};

uint64_t get_MEMORY_MAP_ENTRY_COUNT_ADDRESS() {
    return MEMORY_MAP_ADDRESS - sizeof(uint64_t);
}

uint64_t get_PMM_BITMAP_PHYS_ADDRESS() {
    return get_MEMORY_MAP_ENTRY_COUNT_ADDRESS() - sizeof(uint64_t);
}

uint64_t get_RSDP_ADDRESS_PHYS_ADDRESS() {
    return get_PMM_BITMAP_PHYS_ADDRESS() - sizeof(uint64_t);
}

uint64_t get_GOP_PHYS_ADDRESS() {
    return get_RSDP_ADDRESS_PHYS_ADDRESS() - sizeof(kernel::GOP);
}

uint64_t get_GOP_INFO_PHYS_ADDRESS() {
    return get_GOP_PHYS_ADDRESS() - sizeof(kernel::GOP_Info);
}

uint64_t get_LOWEST_ADDRESS() {
    return kernel::min(MEMORY_MAP_ADDRESS, MEMORY_MAP_ENTRY_COUNT_ADDRESS, PMM_BITMAP_PHYS_ADDRESS, RSDP_ADDRESS_PHYS_ADDRESS, GOP_PHYS_ADDRESS, GOP_INFO_PHYS_ADDRESS);
}

uint64_t GetKernelStackSize() {
    return reinterpret_cast<uint64_t>(stack_top) - reinterpret_cast<uint64_t>(stack_bottom);
}
