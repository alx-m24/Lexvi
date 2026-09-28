#include "common/acpi/rsdp.hpp"

extern "C" {
#include <efi/efi.h>
#include <efi/efilib.h>
}

#include "boot/bootContext.hpp"
#include "common/types/result.hpp"

namespace Lexvi::ACPI {
    RSDP rsdp{};

    bool checksumValid(const void* ptr, uint32_t length) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(ptr);
        uint8_t sum = 0;
        for (uint32_t i = 0; i < length; ++i) sum += bytes[i];
        return sum == 0;
    }

    Types::Result RSDP::Init() {
        EFI_GUID acpi20Guid = ACPI_20_TABLE_GUID;
        EFI_GUID acpi10Guid = ACPI_TABLE_GUID;
    
        RSDP* found20 = nullptr;
        RSDP* found10 = nullptr;
    
        for (UINTN i = 0; i < bootContext.system_table->NumberOfTableEntries; ++i) {
            EFI_CONFIGURATION_TABLE& entry = bootContext.system_table->ConfigurationTable[i];
    
            if (CompareGuid(&entry.VendorGuid, &acpi20Guid)) {
                found20 = reinterpret_cast<RSDP*>(entry.VendorTable);
            } else if (CompareGuid(&entry.VendorGuid, &acpi10Guid)) {
                found10 = reinterpret_cast<RSDP*>(entry.VendorTable);
            }
        }
    
        RSDP* chosen = nullptr;
    
        if (found20 && found20->revision >= 2) {
            // ACPI 2.0+: validate the full extended structure (length field tells you how much)
            if (checksumValid(found20, 20) &&
                checksumValid(reinterpret_cast<const RSDP*>(found20), found20->length)) {
                chosen = found20;
            }
        }
    
        if (!chosen && found10) {
            // ACPI 1.0: only the first 20 bytes are meaningful/checksummed
            if (checksumValid(found10, 20)) {
                chosen = found10;
            }
        }
    
        if (!chosen) {
            return Types::Result{ Types::cstring_view("Failed to find a valid RSDP") };
        }
    
        rsdp = *chosen;

        return {};
    }

    void RSDP::Shutdown() { }
}
