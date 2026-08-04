#pragma once
#include <cstdint>

#include "asm/instructions.hpp"

namespace kernel::serial {
    constexpr uint16_t COM1 = 0x3F8;

    inline void init() {
        outb(COM1 + 1, 0x00); // disable interrupts
        outb(COM1 + 3, 0x80); // enable DLAB
        outb(COM1 + 0, 0x03); // divisor lo -> 38400 baud
        outb(COM1 + 1, 0x00); // divisor hi
        outb(COM1 + 3, 0x03); // 8N1, DLAB off
        outb(COM1 + 2, 0xC7); // enable FIFO
        outb(COM1 + 4, 0x0B); // IRQs off, RTS/DSR set
    }

    inline bool transmitEmpty() {
        return inb(COM1 + 5) & 0x20;
    }

    inline void put(const char& c) {
        while (!transmitEmpty()) {}
        outb(COM1, static_cast<uint8_t>(c));
    }

    inline void put(const char* str) {
        for (; *str; ++str) {
            if (*str == '\n') put('\r'); // CRLF for terminal sanity
            put(*str);
        }
    }

    inline void put(const uint64_t& n) {
        if (n == 0) {
            put('0');
            return;
        }

        char num[20] = {};
        int index = 0;

        uint64_t num_cpy = n;
        while (num_cpy != 0) {
            num[index++] = '0' + (num_cpy % 10);
            num_cpy /= 10;
        }

        for (int i = index - 1; i >= 0; --i) {
            put(num[i]);
        }
 
    }

    inline void put(const uint32_t& n) {
        put(static_cast<uint64_t>(n));
    }

    inline void put(const int32_t& n) {
        if (n < 0) {
            put('-');
            put(static_cast<uint64_t>(-n));
            return;
        }
        put(static_cast<uint64_t>(n));
    }

    inline void putHex(uint64_t n) {
        put("0x");
        for (int i = 15; i >= 0; --i) {
            uint8_t digit = (n >> (i * 4)) & 0xF;
            put(static_cast<char>(digit < 10 ? '0' + digit : 'A' + (digit - 10)));
        }
    }

    inline void put() {}

    template<typename First, typename... Others>
    void put(const First& first, const Others&... others) {
        put(first);
        put(others...);
    }
}

#ifndef NDEBUG
    #define KLOG(...)     kernel::serial::put(__VA_ARGS__)
    #define KLOG_HEX(x)   kernel::serial::putHex(x)
#else
    #define KLOG(...)     do { } while (false)
    #define KLOG_HEX(x)   do { } while (false)
#endif
