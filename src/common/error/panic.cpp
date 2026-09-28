#include "common/error/panic.hpp"

#include "common/output/output-defs.hpp"
#include "common/asm.hpp"

namespace Lexvi::Error {
    void panic(const char* msg, const char *file, int line) {
        LEXVI_PRINT("\nKERNEL PANIC: ");
        LEXVI_PRINT(msg);
        LEXVI_PRINT(" at ", file, ": ", line, '\n');

        // Halt forever
        ASM("cli; hlt");
        __builtin_unreachable();
    }
}
