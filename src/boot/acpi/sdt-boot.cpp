#include "common/acpi/sdt.hpp"

#include "common/acpi/rsdp.hpp"

namespace Lexvi::ACPI {
    SDTHeader* sdtHeader{};
    
    Types::Result SDTHeader::Init() {
        uint64_t sdtAddr = (rsdp.revision >= 2)
            ? rsdp.xsdt_address
            : static_cast<uint64_t>(rsdp.rsdt_address);
    
        SDTHeader* header = reinterpret_cast<SDTHeader*>(sdtAddr);
    
        uint8_t sum = 0;
        uint8_t* bytes = reinterpret_cast<uint8_t*>(header);
        for (uint32_t i = 0; i < header->length; ++i) sum += bytes[i];
        if (sum != 0) {
            return Types::Result{ Types::cstring_view("RSDT/XSDT checksum invalid") };
        }

        sdtHeader = header;
    
        return {};
    }

    void SDTHeader::Shutdown() { }
}
