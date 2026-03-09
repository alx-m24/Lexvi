#include "kernel/error/error.hpp"

#include "kernel/console/console.hpp"

namespace kernel {
    void panic(const char* msg, const char *file, int line) {
        ScopedColor scopedColor(Color::RED_ON_BLACK);

        printf("\nKERNEL PANIC: ");

        setColor(Color::WHITE_ON_BLACK);
        printf(msg);

        setColor(Color::YELLOW_ON_BLACK);
        printf(" at ", file, ": ", line, '\n');

        setColor(Color::WHITE_ON_BLACK);

        // Halt forever
        asm volatile ("cli; hlt");
        __builtin_unreachable();
    }
}
