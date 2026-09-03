#include "kernel/acpi/mcfg.hpp"
#include "kernel/error/error.hpp"
#include "kernel/debug/gop.hpp"

#include "kernel/utils/tables.hpp"

MCFGEntry* mcfg_entries = nullptr;
uint64_t mcfg_count{};

void mcfg_load(kernel::VMM& vmm) {
    MCFGTable* mcfgTable = kernel::findTable<MCFGTable>();

    KERNEL_ASSERT(mcfgTable != nullptr);

    MCFGEntry* entries = reinterpret_cast<MCFGEntry*>(reinterpret_cast<uint8_t*>(mcfgTable) + sizeof(MCFGTable));

    mcfg_entries = entries;

    mcfg_count =
        (mcfgTable->header.length - sizeof(MCFGTable))
        / sizeof(MCFGEntry);

    for (uint64_t i = 0; i < mcfg_count; ++i) {
        const MCFGEntry& entry = mcfg_entries[i];
        KERNEL_ASSERT(entry.segment_group == 0);

        KERNEL_PRINT("          - MCFG Entry ", i, ": BaseAddress");
        KERNEL_PRINTHEX(entry.base_address);
        KERNEL_PRINT(" start_bus=", entry.start_bus, " end_bus=", entry.end_bus, '\n');

        const Bytes size =
            MiB(1).bytes() *
            (entry.end_bus - entry.start_bus + 1);

        vmm.mapMMIO(
            MMIO_TO_VIRT(entry.base_address),
            entry.base_address,
            size
        );
    }
}

const MCFGEntry* mcfg_getEntry(uint8_t bus) {
    KERNEL_ASSERT(mcfg_entries != nullptr);

    for (uint64_t i = 0; i < mcfg_count; ++i) {
        const MCFGEntry& entry = mcfg_entries[i];

        if (bus >= entry.start_bus && bus <= entry.end_bus)
            return &entry;
    }

    KERNEL_PANIC("Bus not found in MCFG entries");
    return nullptr;
}
