#include "kernel/utils/memory.hpp"

extern "C" void* memset(void* dest, int value, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    while (count--) {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}
