#pragma once

// Collection of asm instructions

// #define ASM __asm__ volatile
#define ASM(...) __asm__ volatile (__VA_ARGS__)

namespace Lexvi {
    void halt();
}
