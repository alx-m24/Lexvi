#include "boot/output/print.hpp"

namespace Lexvi::Boot::Output {
    void EFI_Print::print(const char& c) {
        Print((const CHAR16*)u"%c", c);
    }

    void EFI_Print::print(const char* str) {
        Print((const CHAR16*)u"%a", str);
    }

    void EFI_Print::print(const CHAR16* str) {
        Print((const CHAR16*)u"%s", str);
    }

    void EFI_Print::print(const uint64_t& n) {
        Print((const CHAR16*)u"%llu", n);
    }

    void EFI_Print::print(const int64_t& n) {
        Print((const CHAR16*)u"%lld", n);
    }

    void EFI_Print::print(bool n) {
        Print((const CHAR16*)u"%s", n ? (const CHAR16*)u"True" : (const CHAR16*)u"False");
    }

    void EFI_Print::printHex(uint64_t n) {
        Print((const CHAR16*)u"0x%016llX", n);
    }

}
