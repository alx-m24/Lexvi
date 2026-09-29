#include "common/memory/vmm.hpp"

#include "boot/memory/alloc.hpp"
#include "boot/bootContext.hpp"

#include "kernel/header.hpp"

#include "common/memory/memory-map.hpp"
#include "common/memory/pmm.hpp"
#include "common/memory/memory-defs.hpp"
#include "common/memory/address.hpp"
#include "common/utils/memory.hpp"
#include "common/error/assert.hpp"
#include "common/output/output-defs.hpp"

namespace Lexvi::Memory { 
    PageTable* PageTableEntry::getNextPageTable() const {
        PhysicalAddress phys = extractBits(m_raw, 12, 51) << 12;
        return reinterpret_cast<PageTable*>(phys.getValue());
    }

    Types::Result VMM::Init() {
        m_pmm = bootContext.pmm;    
        // 1 page == 4_kb which equal sizeof(PageTable)
        m_pml4Phys = m_pmm->Alloc(1); 
        if (m_pml4Phys.isNull()) {
            return { Types::cstring_view("Failed to allocate PML4") }; 
        }
        m_pml4 = reinterpret_cast<PageTable*>(m_pml4Phys.getValue()); // 1:1 mapping still active
        m_pml4->clear();

        // Kernel image (slot 511)
        uint64_t kernelPhys = Kernel::KERNEL_MAIN_LOAD_ADDR.getValue();  // 0x100000
        uint64_t kernel_Size = alignUp(bootContext.kernelSize.count(), PMM::PAGE_SIZE.bytes().count());

        for (uint64_t off = 0; off < kernel_Size; off += PMM::PAGE_SIZE.bytes().count()) {
            map(KERNEL_VIRT_BASE.bytes().count() + off, kernelPhys + off, { .writable = true });
        }

        for (uint32_t i = 0; i < bootContext.MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = bootContext.E820Entries[i];

            uint64_t base = alignDown(entry.base, MiB(2).bytes().count());
            uint64_t end  = alignUp(entry.base + entry.length, MiB(2).bytes().count());

            for (uint64_t phys = base; phys < end; phys += MiB(2).bytes().count()) {
                map(
                    HHDM_BASE.bytes().count() + phys,
                    phys,
                    { 
                        .writable = true,
                        .hugePage = true,
                        .cacheDisable = entry.type == EntryType::Usable ? false : true
                    });
                if (phys <= 0x100000) continue; // prevents double mapping of lower 1MB using 2 different page sizes
                if (entry.type == EntryType::Usable) map(
                    phys,
                    phys,
                    { 
                        .writable = true,
                        .hugePage = true,
                        .cacheDisable = entry.type == EntryType::Usable ? false : true
                    });
            }
        }

        return {};
    }

    void VMM::map(uint64_t virt, uint64_t phys, PageFlags flags) {
        auto idx = [](uint64_t v, int shift) -> uint64_t {
            return (v >> shift) & 0x1FF;
        };

        auto getOrAlloc = [&](PageTable* table, uint64_t index) -> PageTable* {
            PageTableEntry& entry = table->entries[index];
            if (!entry.isPresent()) {
                PhysicalAddress newPhys = m_pmm->Alloc(1);
                PageTable* newTable = reinterpret_cast<PageTable*>(newPhys.getValue());
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
