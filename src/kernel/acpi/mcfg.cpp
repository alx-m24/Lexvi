#include "kernel/acpi/mcfg.hpp"
#include "kernel/error/error.hpp"

#include "kernel/utils/tables.hpp"

uint64_t mcfg_base = 0;

void mcfg_load() {
    MCFGTable* mcfgTable = kernel::findTable<MCFGTable>("MCFG");
    KERNEL_ASSERT(mcfgTable != nullptr);
    MCFGEntry* entries = reinterpret_cast<MCFGEntry*>(reinterpret_cast<uint8_t*>(mcfgTable) + sizeof(MCFGTable));
    mcfg_base = entries[0].base_address;
    mcfg_count = (mcfgTable->header.length - sizeof(MCFGTable)) / sizeof(MCFGEntry);
}
