#include "common/asm.hpp"

namespace Lexvi {
    void halt() {
        ASM("cli; hlt");
    }
}
