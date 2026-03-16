#include "kernel/acpi/hpet.hpp"
#include "kernel/acpi/rsdp.hpp"
#include "kernel/acpi/sdt.hpp"
#include "kernel/error/error.hpp"

void hpet_load() {
    KERNEL_ASSERT(sdtHeader != nullptr);

    constexpr char HPET_SIGNATURE[] = "HPET";

    bool isXsdt = rsdp.revision >= 2;
    const char* expected = isXsdt ? "XSDT" : "RSDT";

    uint32_t pointer_size = isXsdt ? 8 : 4;
    uint32_t entries = (sdtHeader->length - sizeof(SDTHeader)) / pointer_size;

    uint8_t* entry_ptr = reinterpret_cast<uint8_t*>(sdtHeader + sizeof(SDTHeader));
    for (uint32_t i = 0; i < entries; ++i) {
        uint64_t entry_addr = isXsdt
            ? *reinterpret_cast<uint64_t*>(entry_ptr + i * 8)
            : *reinterpret_cast<uint32_t*>(entry_ptr + i * 4);

        SDTHeader* entry = reinterpret_cast<SDTHeader*>(entry_addr);
        bool isHPET = true;
        for (int j = 0; j < 4; ++j) {
            if (entry->signature[j] != HPET_SIGNATURE[j]) {
                isHPET = false;
                break;
            }
        }

        if (isHPET) {
            HPETTable* hpet_table = reinterpret_cast<HPETTable*>(entry_addr);
            uint64_t hpet_base = hpet_table->address;
            return;
        }
    }

}
