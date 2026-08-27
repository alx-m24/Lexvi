#include "kernel/error/error.hpp"

#include "kernel/debug/gop.hpp"

namespace kernel {
    void panic(const char* msg, const char *file, int line) {
        KERNEL_PRINT("\nKERNEL PANIC: ");
        KERNEL_PRINT(msg);
        KERNEL_PRINT(" at ", file, ": ", line, '\n');

        // Halt forever
        asm volatile ("cli; hlt");
        __builtin_unreachable();
    }
}
