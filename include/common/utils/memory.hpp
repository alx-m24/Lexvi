#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>

#include "common/error/assert.hpp"

extern "C" inline void* memset(void* dest, int value, size_t count) {
    uint8_t* ptr = (uint8_t*)dest;
    while (count--) {
        *ptr++ = (uint8_t)value;
    }
    return dest;
}

namespace Lexvi::Memory {
    inline constexpr uint64_t extractBits(
        uint64_t value,
        uint8_t startBit,
        uint8_t endBit
    ) {
        LEXVI_ASSERT(startBit <= endBit);
        LEXVI_ASSERT(endBit < 64);
    
        const uint8_t bitCount = endBit - startBit + 1;
    
        // Special case: full 64-bit mask
        if (bitCount == 64) {
            return value;
        }
    
        const uint64_t mask = (1ULL << bitCount) - 1;
    
        return (value >> startBit) & mask;
    }
    
    inline uint64_t alignDown(uint64_t value, uint64_t align) {
        return value & ~(align - 1);
    }
    
    inline uint64_t alignUp(uint64_t value, uint64_t align) {
        return (value + align - 1) & ~(align - 1);
    }
}
