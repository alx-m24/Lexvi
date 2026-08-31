#include "kernel/memory/internals/vmm.hpp"

#include "kernel/memory/internals/memory-map.hpp"
#include "kernel/memory/internals/pmm.hpp"
#include "kernel/memory/memory-defs.hpp"
#include "kernel/kernel-config.hpp"
#include "kernel/utils/memory.hpp"
#include "kernel/error/error.hpp"
#include "kernel/debug/gop.hpp"

namespace kernel { 
    PageTable* PageTableEntry::getNextPageTable() const {
        uint64_t phys = extractBits(m_raw, 12, 51) << 12;
        return reinterpret_cast<PageTable*>(TO_VIRT(phys));
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

#ifndef BOOTLOADER
    void VMM::Init(PageTable* existingPML4, PMM& pmm) {
        m_pmm = &pmm;
        m_pml4 = existingPML4;
        m_pml4Phys = TO_PHYS(existingPML4);

        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];

            uint64_t base = alignDown(entry.base, MiB(2).bytes().count());
            uint64_t end  = alignUp(entry.base + entry.length, MiB(2).bytes().count());

            for (uint64_t phys = base; phys < end; phys += MiB(2).bytes().count()) {
                if (entry.type != EntryType::Usable) continue;
                unmap(phys);
            }
        }
    }
#else
    void VMM::Init(PMM& pmm, Bytes kernelSize, Bytes ImageBase, Bytes ImageSize) {
        m_pmm = &pmm;    
        m_pml4Phys = TO_PHYS(reinterpret_cast<uint64_t>(m_pmm->Alloc(1))); // 1 page == 4_kb which equal sizeof(PageTable)
        m_pml4 = reinterpret_cast<PageTable*>(TO_VIRT(m_pml4Phys));
        m_pml4->clear();


        // Kernel image (slot 511)
        uint64_t kernelPhys = KERNEL_MAIN_LOAD_ADDR;  // 0x100000
        uint64_t kernel_Size = alignUp(kernelSize.count(), PAGE_SIZE.bytes().count());

        for (uint64_t off = 0; off < kernel_Size; off += PAGE_SIZE.bytes().count()) {
            map(KERNEL_VIRT_BASE + off, kernelPhys + off, { .writable = true });
        }


        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];

            uint64_t base = alignDown(entry.base, MiB(2).bytes().count());
            uint64_t end  = alignUp(entry.base + entry.length, MiB(2).bytes().count());

            for (uint64_t phys = base; phys < end; phys += MiB(2).bytes().count()) {
                map(
                    HHDM_BASE + phys,
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

        for (uint64_t phys = 0; phys < 0x100000; phys += PAGE_SIZE.bytes().count()) {
            map(phys, phys, { .writable = true });
        }

        uint64_t imageBase = alignDown(ImageBase.count(), PAGE_SIZE.bytes().count());
        uint64_t imageEnd = alignUp(ImageBase.count() + ImageSize.count(), PAGE_SIZE.bytes().count());
        for (uint64_t phys = imageBase; phys < imageEnd; phys += PAGE_SIZE.bytes().count()) {
            map(phys, phys, { .writable = true });
        }

        loadCR3();
    }
#endif
    
#ifndef BOOTLOADER
    void VMM::map(uint64_t virt, uint64_t phys, PageFlags flags) {
        auto idx = [](uint64_t v, int shift) -> uint64_t {
            return (v >> shift) & 0x1FF;
        };

        auto getOrAlloc = [&](PageTable* table, uint64_t index) -> PageTable* {
            PageTableEntry& entry = table->entries[index];
            if (!entry.isPresent()) {
                uint64_t newPhys = TO_PHYS(reinterpret_cast<uint64_t>(m_pmm->Alloc(1)));
                PageTable* newTable = reinterpret_cast<PageTable*>(TO_VIRT(newPhys));
                newTable->clear();
                entry.set<{ .writable = true }>(newPhys);
            }
            return entry.getNextPageTable();
        };

        PageTable* pdpt = getOrAlloc(m_pml4,  idx(virt, 39));
        PageTable* pd   = getOrAlloc(pdpt,    idx(virt, 30));

        if (flags.hugePage) {
            KERNEL_ASSERT((virt & (MiB(2).bytes().count() - 1)) == 0);
            KERNEL_ASSERT((phys & (MiB(2).bytes().count() - 1)) == 0);

            PageTableEntry& entry = pd->entries[idx(virt, 21)];

            if (entry.isPresent()) {
                if (entry.isHugePage()) {
                    if (entry.getPhys() != phys) {
                        KERNEL_PRINT("MAP COLLISION virt="); KERNEL_PRINTHEX(virt);
                        KERNEL_PRINT(" wanted="); KERNEL_PRINTHEX(phys);
                        KERNEL_PRINT(" existing="); KERNEL_PRINTHEX(entry.getPhys());
                        KERNEL_PRINT('\n');
                    }
                    KERNEL_ASSERT(entry.getPhys() == phys);
                    return;
                }
            
                KERNEL_PANIC("Cannot allocate huge page where small pages already exist");
            }
            
            entry.set(phys, flags);
        }
        else {
            KERNEL_ASSERT((virt & (PAGE_SIZE.bytes().count() - 1)) == 0);
            KERNEL_ASSERT((phys & (PAGE_SIZE.bytes().count() - 1)) == 0);

            PageTableEntry& pdEntry = pd->entries[idx(virt, 21)];
            if (pdEntry.isPresent() && pdEntry.isHugePage()) {
                KERNEL_PANIC("Cannot allocate small page inside existing huge page");
            }

            PageTable* pt   = getOrAlloc(pd,      idx(virt, 21));
            PageTableEntry& entry = pt->entries[idx(virt, 12)];

            if (entry.isPresent()) {
                if (entry.getPhys() != phys) {
                    KERNEL_PRINT("MAP COLLISION virt="); KERNEL_PRINTHEX(virt);
                    KERNEL_PRINT(" wanted="); KERNEL_PRINTHEX(phys);
                    KERNEL_PRINT(" existing="); KERNEL_PRINTHEX(entry.getPhys());
                    KERNEL_PRINT('\n');
                }
                KERNEL_ASSERT(entry.getPhys() == phys);
                return;
            }
            else {
                entry.set(phys, flags);
            }
        }
    }

#else
    void VMM::map(uint64_t virt, uint64_t phys, PageFlags flags) {
        auto idx = [](uint64_t v, int shift) -> uint64_t {
            return (v >> shift) & 0x1FF;
        };

        auto getOrAlloc = [&](PageTable* table, uint64_t index) -> PageTable* {
            PageTableEntry& entry = table->entries[index];
            if (!entry.isPresent()) {
                uint64_t newPhys = TO_PHYS(reinterpret_cast<uint64_t>(m_pmm->Alloc(1)));
                PageTable* newTable = reinterpret_cast<PageTable*>(TO_VIRT(newPhys));
                newTable->clear();
                entry.set<{ .writable = true }>(newPhys);
            }
            return entry.getNextPageTable();
        };

        PageTable* pdpt = getOrAlloc(m_pml4,  idx(virt, 39));
        PageTable* pd   = getOrAlloc(pdpt,    idx(virt, 30));

        if (flags.hugePage) {
            pd->entries[idx(virt, 21)].set(phys, flags);
        }
        else {
            PageTable* pt   = getOrAlloc(pd,      idx(virt, 21));
            pt->entries[idx(virt, 12)].set(phys, flags);
        }
    }

#endif

    void VMM::mapMMIO(uint64_t virtBase, uint64_t physBase, Bytes size) {
        uint64_t pages = alignUp(size.count(), PAGE_SIZE.bytes().count()) / PAGE_SIZE.bytes().count();
        for (uint64_t i = 0; i < pages; i++) {
            map(virtBase + i * PAGE_SIZE.bytes().count(),
                physBase + i * PAGE_SIZE.bytes().count(),
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
        void* phys = m_pmm->Alloc(pageCount);
        if (phys == nullptr) return nullptr;

        uint64_t physBase = TO_PHYS(phys);
        
        for (uint64_t i = 0; i < pageCount; ++i) {
            map(virt + i * PAGE_SIZE.bytes().count(),
                physBase + i * PAGE_SIZE.bytes().count(),
                flags);
        }
        return reinterpret_cast<void*>(virt);
    }

    void VMM::loadCR3() {
        asm volatile("mov %0, %%cr3" :: "r"(m_pml4Phys) : "memory");
    }
}
