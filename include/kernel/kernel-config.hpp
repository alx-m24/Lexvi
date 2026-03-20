// This is an auto-generated header file from the build.sh script

#pragma once

#include "stdint.h"

constexpr unsigned int KERNEL_MAIN_LBA = 9;
constexpr unsigned int KERNEL_MAIN_SECTORS = 79;
constexpr unsigned long long KERNEL_MAIN_LOAD_ADDR = 0x100000;

constexpr unsigned long long TEMP_KERNEL_MAIN_LOAD_ADDR = 0x10000;

constexpr unsigned int MEMORY_MAP_ADDRESS = 0x7000;
constexpr unsigned int MEMORY_MAP_ENTRY_COUNT_ADDRESS = 0x6FF8;

extern "C" {
    extern char stack_top[];
    extern char stack_bottom[];
    extern char _kernel_end[];
}

inline uint64_t GetKernelStackSize() {
    return reinterpret_cast<uint64_t>(stack_top) - reinterpret_cast<uint64_t>(stack_bottom);
}
