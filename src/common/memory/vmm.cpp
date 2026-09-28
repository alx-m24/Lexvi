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
        return reinterpret_cast<PageTable*>(phys.TO_HHDM().getPtr()); // TODO: Boot -> Does NOT use HHDM
    }
    
    bool PageTableEntry::isPresent() const { 
        return m_raw & 1;
    }

    bool PageTableEntry::isHugePage() const {
        return m_raw & (1ULL << 7);
    }
    
    void PageTableEntry::set(uint64_t physAddr, PageFlags flags) {
        m_raw = (physAddr & 0x000FFFFFFFFFF000ULL) | flags();
    }
    
    void PageTableEntry::clear() { 
        m_raw = 0;
    }
    
    uint64_t PageTableEntry::getPhys() const {
        return extractBits(m_raw, 12, 51) << 12;
    }

    void PageTable::clear() {
        memset(entries, 0, sizeof(entries));
    }

    void VMM::map(uint64_t virt, uint64_t phys, PageFlags flags) {
        auto idx = [](uint64_t v, int shift) -> uint64_t {
            return (v >> shift) & 0x1FF;
        };

        auto getOrAlloc = [&](PageTable* table, uint64_t index) -> PageTable* {
            PageTableEntry& entry = table->entries[index];
            if (!entry.isPresent()) {
                PhysicalAddress newPhys = (reinterpret_cast<uint64_t>(m_pmm->Alloc(1)));
                PageTable* newTable = newPhys.TO_HHDM().getPtr(); // TODO -> Bootloader NOT USE VIRT
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

    void VMM::mapMMIO(uint64_t virtBase, uint64_t physBase, Bytes size) {
        uint64_t pages = alignUp(size.count(), PMM::PAGE_SIZE.bytes().count()) / PMM::PAGE_SIZE.bytes().count();
        for (uint64_t i = 0; i < pages; i++) {
            map(virtBase + i * PMM::PAGE_SIZE.bytes().count(),
                physBase + i * PMM::PAGE_SIZE.bytes().count(),
                { .writable = true, .cacheDisable = true }); // cache disable important for MMIO
        }
    }

    void VMM::unmap(uint64_t virt) {
        auto idx = [](uint64_t v, int shift) -> uint64_t {
            return (v >> shift) & 0x1FF;
        };
    
        PageTableEntry& pml4e = m_pml4->entries[idx(virt, 39)];
        if (!pml4e.isPresent()) return;
    
        PageTableEntry& pdpte = pml4e.getNextPageTable()->entries[idx(virt, 30)];
        if (!pdpte.isPresent()) return;
    
        PageTableEntry& pde = pdpte.getNextPageTable()->entries[idx(virt, 21)];
        if (!pde.isPresent()) return;

        if (pde.isHugePage()) {
            pde.clear();
        }
        else {
            PageTableEntry& pte = pde.getNextPageTable()->entries[idx(virt, 12)];
            pte.clear();
        }
        // TLB shootdown needed here on SMP; for now, single-core invlpg suffices
        asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
    }

    void* VMM::Alloc(uint64_t virt, uint64_t pageCount, PageFlags flags) {
        PhysicalAddress phys = m_pmm->Alloc(pageCount);
        if (phys.isNull()) return nullptr;

        for (uint64_t i = 0; i < pageCount; ++i) {
            map(virt + i * PMM::PAGE_SIZE.bytes().count(),
                phys.getValue() + i * PMM::PAGE_SIZE.bytes().count(),
                flags);
        }
        return reinterpret_cast<void*>(virt);
    }

    void VMM::loadCR3() {
        asm volatile("mov %0, %%cr3" :: "r"(m_pml4Phys) : "memory");
    }
}
