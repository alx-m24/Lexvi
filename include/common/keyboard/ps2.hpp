#pragma once

#include "common/types/result.hpp"

namespace Lexvi::Keyboard {
    struct PS2 {
        static Types::Result Init();
        static void Shutdown();
    };
}
