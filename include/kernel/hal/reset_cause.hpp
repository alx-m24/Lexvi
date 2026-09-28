#pragma once

#include <cstdint>

namespace Lexvi::Kernel::hal::pwrm {
    enum class ResetCause : uint8_t {
        NONE = 0,
        COUNT
    };

    ResetCause getResetCause();
}
