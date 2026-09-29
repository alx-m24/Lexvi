#include "common/memory/vmm.hpp"

#include "boot/memory/alloc.hpp"
#include "boot/bootContext.hpp"

#include "common/asm.hpp"
#include "common/memory/pmm.hpp"
#include "common/memory/address.hpp"
#include "common/utils/memory.hpp"

namespace Lexvi::Memory { 
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
        ASM("invlpg (%0)" : : "r"(virt) : "memory");
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
        ASM("mov %0, %%cr3" :: "r"(m_pml4Phys) : "memory");
    }
}
