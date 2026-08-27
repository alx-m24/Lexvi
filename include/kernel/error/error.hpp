#pragma once

namespace kernel {
    void panic(const char* msg, const char *file, int line);
}

#define KERNEL_PANIC(msg) kernel::panic(msg, __FILE__, __LINE__)

#ifdef NDEBUG
#define KERNEL_ASSERT(cond) do { } while (false)
#else
#define KERNEL_ASSERT(cond) \
        if (!(cond)) { KERNEL_PANIC("Assertion Failed: " #cond); }
#endif
