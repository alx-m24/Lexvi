#pragma once

namespace kernel {
    void panic(const char* msg, const char *file, int line);
}

#define KERNEL_PANIC(msg) kernel::panic(msg, __FILE__, __LINE__)
#define KERNEL_ASSERT(cond) \
        if (!(cond)) { KERNEL_PANIC("Assertion Failed: " #cond); }
