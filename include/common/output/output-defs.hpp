#pragma once

#ifdef NDEBUG

#define LEXVI_PRINT(...) do { } while (false)
#define LEXVI_PRINTHEX(x) do { } while (false)

#else

#ifdef BOOTLOADER

#include "boot/output/print.hpp"

#define LEXVI_PRINT(...) do { \
    Lexvi::Boot::Output::EFI_Print::print(__VA_ARGS__); \
} while (false)

#define LEXVI_PRINTHEX(x) do { \
    Lexvi::Boot::Output::EFI_Print::printHex(x); \
} while (false)

#else

#include "common/output/gop.hpp"

#define LEXVI_PRINT(...) do { \
    Lexvi::Output::GOP::print(__VA_ARGS__); \
} while (false)

#define LEXVI_PRINTHEX(x) do { \
    Lexvi::Output::GOP::printHex(static_cast<uint64_t>(x)); \
} while (false)

#endif

#endif
