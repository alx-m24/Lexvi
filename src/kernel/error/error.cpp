#include "kernel/error/error.hpp"

#include "kernel/io/console.hpp"

namespace kernel {
    void panic(const char* msg, const char *file, int line) {
        printf("\nKERNEL PANIC: ", msg, " at ", file, ": ", line, '\n');

        // Halt forever
        asm volatile ("cli; hlt");
        __builtin_unreachable();
    }
}
