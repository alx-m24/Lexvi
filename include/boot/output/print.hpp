#pragma once

#include <cstdint>
#include <concepts>
#include <type_traits>

extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

namespace Lexvi::Boot::Output {

    struct EFI_Print {
        static void print(const char& c);
        static void print(const char* str);
        static void print(const CHAR16* str);

        static void print(const uint64_t& n);
        static void print(const int64_t& n);

        template<std::integral T>
            requires (!std::same_as<T, char> && !std::same_as<T, bool>)
        static void print(T n) {
            if constexpr (std::is_signed_v<T>)
                print(static_cast<int64_t>(n));
            else
                print(static_cast<uint64_t>(n));
        }

        static void print(bool n);

        static void printHex(uint64_t n);

        static void print() {}

        template<typename First, typename... Others>
        static void print(const First& first, const Others&... others) {
            EFI_Print::print(first);
            EFI_Print::print(others...);
        }
    };

}
