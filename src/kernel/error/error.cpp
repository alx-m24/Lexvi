#include "kernel/error/error.hpp"

#include "kernel/debug/serial.hpp"

namespace kernel {
    void panic(const char* msg, const char *file, int line) {
        kernel::serial::put("\nKERNEL PANIC: ");
        kernel::serial::put(msg);
        kernel::serial::put(" at ", file, ": ", line, '\n');

        // Halt forever
        asm volatile ("cli; hlt");
        __builtin_unreachable();
    }
}
