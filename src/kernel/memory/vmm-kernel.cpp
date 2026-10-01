#include "common/memory/vmm.hpp"

#include "boot/memory/alloc.hpp"
#include "boot/bootContext.hpp"

#include "common/memory/pmm.hpp"
#include "common/memory/address.hpp"
#include "common/utils/memory.hpp"
#include "common/error/assert.hpp"
#include "common/output/output-defs.hpp"

namespace Lexvi::Memory { 
    PageTable* PageTableEntry::getNextPageTable() const {
        PhysicalAddress phys = extractBits(m_raw, 12, 51) << 12;
        return reinterpret_cast<PageTable*>(phys.TO_HHDM().getPtr());
    }

    Types::Result VMM::Init() {
        // Read PMM and existing page tables from bootInfo
        // m_pmm = &pmm;
        // m_pml4 = existingPML4;
        // m_pml4Phys = TO_PHYS(existingPML4);

        // for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
        //     const E820Entry entry = E820Entries[i];

        //     uint64_t base = alignDown(entry.base, MiB(2).bytes().count());
        //     uint64_t end  = alignUp(entry.base + entry.length, MiB(2).bytes().count());

        //     for (uint64_t phys = base; phys < end; phys += MiB(2).bytes().count()) {
        //         if (entry.type != EntryType::Usable) continue;
        //         unmap(phys);
        //     }
        // }
    }

    void VMM::map(uint64_t virt, uint64_t phys, PageFlags flags) {
        auto idx = [](uint64_t v, int shift) -> uint64_t {
            return (v >> shift) & 0x1FF;
        };

        auto getOrAlloc = [&](PageTable* table, uint64_t index) -> PageTable* {
            PageTableEntry& entry = table->entries[index];
            if (!entry.isPresent()) {
                PhysicalAddress newPhys = m_pmm->Alloc(1);
                PageTable* newTable = newPhys.TO_HHDM().getPtr<PageTable>();
                newTable->clear();
                entry.set<{ .writable = true }>(newPhys);
            }
            return entry.getNextPageTable();
        };

        PageTable* pdpt = getOrAlloc(m_pml4,  idx(virt, 39));
        PageTable* pd   = getOrAlloc(pdpt,    idx(virt, 30));

        if (flags.hugePage) {
            LEXVI_ASSERT((virt & (MiB(2).bytes().count() - 1)) == 0);
            LEXVI_ASSERT((phys & (MiB(2).bytes().count() - 1)) == 0);

            PageTableEntry& entry = pd->entries[idx(virt, 21)];

            if (entry.isPresent()) {
                if (entry.isHugePage()) {
                    if (entry.getPhys() != phys) {
                        LEXVI_PRINT("MAP COLLISION virt="); LEXVI_PRINTHEX(virt);
                        LEXVI_PRINT(" wanted="); LEXVI_PRINTHEX(phys);
                        LEXVI_PRINT(" existing="); LEXVI_PRINTHEX(entry.getPhys());
                        LEXVI_PRINT('\n');
                    }
                    LEXVI_ASSERT(entry.getPhys() == phys);
                    return;
                }
            
                LEXVI_PANIC("Cannot allocate huge page where small pages already exist");
            }
            
            entry.set(phys, flags);
        }
        else {
            LEXVI_ASSERT((virt & (PMM::PAGE_SIZE.bytes().count() - 1)) == 0);
            LEXVI_ASSERT((phys & (PMM::PAGE_SIZE.bytes().count() - 1)) == 0);

            PageTableEntry& pdEntry = pd->entries[idx(virt, 21)];
            if (pdEntry.isPresent() && pdEntry.isHugePage()) {
                LEXVI_PANIC("Cannot allocate small page inside existing huge page");
            }

            PageTable* pt   = getOrAlloc(pd,      idx(virt, 21));
            PageTableEntry& entry = pt->entries[idx(virt, 12)];

            if (entry.isPresent()) {
                if (entry.getPhys() != phys) {
                    LEXVI_PRINT("MAP COLLISION virt="); LEXVI_PRINTHEX(virt);
                    LEXVI_PRINT(" wanted="); LEXVI_PRINTHEX(phys);
                    LEXVI_PRINT(" existing="); LEXVI_PRINTHEX(entry.getPhys());
                    LEXVI_PRINT('\n');
                }
                LEXVI_ASSERT(entry.getPhys() == phys);
                return;
            }
            else {
                entry.set(phys, flags);
            }
        }
    }
}
