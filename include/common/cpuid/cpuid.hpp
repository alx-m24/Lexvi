#pragma once

#include "common/asm.hpp"
#include "common/cpuid/vendor.hpp"
#include "common/utils/string.hpp"

#include <cstdint>

namespace Lexvi {
    template<uint8_t Leaf>
    struct CPUID {
        CPUID() = default;
    };

    template<>
    struct CPUID<0> {
        Vendor vendor{};

        CPUID() {
            uint32_t ebx, ecx, edx;
            ASM("cpuid" : "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0) : );

            char id[13] = {};
            *reinterpret_cast<uint32_t*>(&id[0]) = ebx;
            *reinterpret_cast<uint32_t*>(&id[4]) = edx;
            *reinterpret_cast<uint32_t*>(&id[8]) = ecx;

            if (equalsN(id, "GenuineIntel", 12)) vendor = Vendor::INTEL;
            if (equalsN(id, "AuthenticAMD", 12)) vendor = Vendor::AMD;
        }
    };
}
