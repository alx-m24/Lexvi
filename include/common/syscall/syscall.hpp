#pragma once

#include "common/interrupt/frame.hpp"

#include <cstdint>

namespace Lexvi::Interrupt {
    uint64_t handle_syscall(const Frame& frame);
}
