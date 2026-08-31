#include "kernel/memory/internals/pmm.hpp"

#include "kernel/kernel-config.hpp"
#include "kernel/utils/math.hpp"
#include "kernel/utils/memory.hpp"
#include "kernel/memory/memory-defs.hpp"
#include "kernel/memory/memory-unit.hpp"
#include "kernel/memory/internals/memory-map.hpp"

namespace kernel { 

#ifndef BOOTLOADER
    void PMM::Init() {
        getTotalPageNum();
    
        uint64_t bitmapPhys = *reinterpret_cast<uint64_t*>(TO_VIRT(PMM_BITMAP_PHYS_ADDRESS));
        m_bitMap = reinterpret_cast<uint8_t*>(TO_VIRT(bitmapPhys));
    }
#else
    void PMM::Init(Bytes kernelSize, Bytes ImageBase, Bytes ImageSize) {
        getTotalPageNum();

        m_kernelSize = kernelSize;
        m_ImageBase = ImageBase;
        m_ImageSize = ImageSize;

        Bytes kernelPhys = Bytes(KERNEL_MAIN_LOAD_ADDR);  // 0x100000
        
        InitBitMap(kernelPhys + kernelSize);
        *reinterpret_cast<uint64_t*>(PMM_BITMAP_PHYS_ADDRESS) = reinterpret_cast<uint64_t>(m_bitMap);

        CleanBitMap();
    }
#endif

    void PMM::getTotalPageNum() {
        uint32_t* raw = reinterpret_cast<uint32_t*>(MEMORY_MAP_ENTRY_COUNT_ADDRESS);

        Bytes highestAddress = 0_B;
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;

            Bytes end = Bytes(entry.base + entry.length);
            highestAddress = max(end, highestAddress);
        }

        m_totalMemory = highestAddress;

        m_totalPageNum = m_totalMemory.align_up(PAGE_SIZE) / PAGE_SIZE;
        m_bitMapSize = (m_totalPageNum + 7) / 8;
    }

    void PMM::InitBitMap(Bytes reservedEnd) {
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;
    
            uint64_t regionEnd = entry.base + entry.length;
            uint64_t candidate = max(entry.base, reservedEnd.count());
            if (candidate >= regionEnd) continue;
            
            uint64_t available = regionEnd - candidate;
            if (available >= m_bitMapSize) {
                m_bitMap = reinterpret_cast<uint8_t*>(candidate);  // raw physical
                break;
            }
        }
    }

    void PMM::CleanBitMap() {
        ZeroBitMap();
        MarkUsedPages();
    }

    void PMM::ZeroBitMap() {
        memset(m_bitMap, 0xFF, m_bitMapSize); // set all entries as used

        // free pages
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];

            if (entry.type != EntryType::Usable) continue;

            MarkRangeFree(Bytes(entry.base), Bytes(entry.length));
        }
    }

    void PMM::MarkUsedPages() {
        MarkRangeUsed(0_B, 0x100000_B);
    
#ifndef BOOTLOADER
        uint64_t kernelEndPhys = KERN_TO_PHYS(reinterpret_cast<uint64_t>(_kernel_end));
        MarkRangeUsed(0x100000_B, Bytes(kernelEndPhys));
    
        uint64_t bitMapPhys = TO_PHYS(reinterpret_cast<uint64_t>(m_bitMap));
        MarkRangeUsed(Bytes(bitMapPhys), Bytes(bitMapPhys + m_bitMapSize));
#else
        MarkRangeUsed(Bytes(0),
                      Bytes(KERNEL_MAIN_LOAD_ADDR) + m_kernelSize);

        MarkRangeUsed(m_ImageBase, m_ImageBase + m_ImageSize);
    
        MarkRangeUsed(Bytes(reinterpret_cast<uint64_t>(m_bitMap)),
                      Bytes(reinterpret_cast<uint64_t>(m_bitMap) + m_bitMapSize));
#endif
    }


    void PMM::MarkRangeFree(Bytes base, Bytes length) {
        const uint64_t startPage = base.align_up(PAGE_SIZE) / PAGE_SIZE;   // ceil start - skip partial first page
        const uint64_t endPage   = (base + length) / PAGE_SIZE;            // floor end  - don't free partial last page
        for (uint64_t i = startPage; i < endPage; ++i) ClearBit(i);
    }
    
    void PMM::MarkRangeUsed(Bytes base, Bytes end) {
        const uint64_t startPage = base / PAGE_SIZE;                        // floor start - catch partial first page
        const uint64_t endPage   = end.align_up(PAGE_SIZE) / PAGE_SIZE;    // ceil end - catch partial last page
        for (uint64_t i = startPage; i < endPage; ++i) SetBit(i);
    }

    void PMM::SetBit(uint64_t page) {
        if (page >= m_totalPageNum) return;
        const Page index = getPage(page);
        m_bitMap[index.byte] |= (1 << index.bit);
    }

    void PMM::ClearBit(uint64_t page) {
        if (page >= m_totalPageNum) return;
        const Page index = getPage(page);
        m_bitMap[index.byte] &= ~(1 << index.bit);
    }

    bool PMM::TestBit(uint64_t page) const {
        if (page >= m_totalPageNum) return true; // out of bounds = treat as used
        const Page index = getPage(page);
        return m_bitMap[index.byte] & (1 << index.bit);
    }


    void* PMM::Alloc(uint64_t pages) {
        for (uint64_t i = 0; i < m_totalPageNum; ++i) {
            bool found = true;
            for (uint64_t j = 0; j < pages && found; ++j) {
                found = !TestBit(i + j); // TestBit() return TRUE if used -> FOUND should be true when EMPTY
            }

            if (found) {
                const Bytes startPageAddress = PAGE_SIZE.bytes() * i;
                MarkRangeUsed(startPageAddress, startPageAddress + (PAGE_SIZE.bytes() * pages));
                return reinterpret_cast<void*>(TO_VIRT(startPageAddress.count()));
            }
        }
        return nullptr;
    }

    void PMM::Free(void* address) {
        MarkRangeFree(Bytes(TO_PHYS(reinterpret_cast<uint64_t>(address))), PAGE_SIZE.bytes());
    }
}
