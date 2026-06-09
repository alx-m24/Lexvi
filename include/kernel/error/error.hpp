#pragma once

namespace kernel {
    void panic(const char* msg, const char *file, int line);
}

#ifdef BOOTLOADER
#define KERNEL_ASSERT(cond) \
    if (!(cond)) { while (true); };
#define KERNEL_PANIC(msg) do { } while (false)
#else
#define KERNEL_PANIC(msg) kernel::panic(msg, __FILE__, __LINE__)
#define KERNEL_ASSERT(cond) \
        if (!(cond)) { KERNEL_PANIC("Assertion Failed: " #cond); }
#endif
