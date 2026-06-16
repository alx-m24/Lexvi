#include "kernel/acpi/hpet.hpp"
#include "kernel/error/error.hpp"

#include "kernel/utils/tables.hpp"

uint64_t hpet_base = 0;

void hpet_load() {
    HPETTable* hpet = kernel::findTable<HPETTable>("HPET");
    KERNEL_ASSERT(hpet != nullptr);
    hpet_base = hpet->address;
}
