#include "kernel/memory/pmm.hpp"

#include "kernel/utils/math.hpp"
#include "kernel/error/error.hpp"
#include "kernel/utils/memory.hpp"
#include "kernel/memory/memory-window.hpp"

#include <stdint.h>

namespace kernel {
    void PMM::Init(MemoryWindow* window) {
        KERNEL_ASSERT(window != nullptr);
        this->m_window = window;

        ResetPageTable();
        ResetBitMap();
    }


    void PMM::ResetPageTable() {
        m_window->printf("Step 1: finding highest address\n");

        uint64_t highestAddress = 0;
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;
            uint64_t end = entry.base + entry.length;
            if (end > highestAddress) highestAddress = end;
        }
        m_window->printf("highestAddress="); m_window->printfHex(highestAddress); m_window->printf("\n");
    
        m_window->printf("Step 2: computing pdNum\n");

        pd_s = pdpt + 512;
        pdNum = (highestAddress + 0x3FFFFFFF) / 0x40000000;
        PageTables_End_Address = reinterpret_cast<uint64_t>(pd_s + pdNum * pdEntriesNum);

        m_window->printf("pdNum=", pdNum, "\n");
        m_window->printf("pml4="); m_window->printfHex(reinterpret_cast<uint64_t>(pml4)); m_window->printf("\n");
        m_window->printf("pdpt="); m_window->printfHex(reinterpret_cast<uint64_t>(pdpt)); m_window->printf("\n");
        m_window->printf("pd_s="); m_window->printfHex(reinterpret_cast<uint64_t>(pd_s)); m_window->printf("\n");
    
        m_window->printf("Step 3: zeroing pml4\n");
        memset(pml4, 0, PAGE_SIZE);

        m_window->printf("Step 4: zeroing pdpt\n");
        memset(pdpt, 0, PAGE_SIZE);

        m_window->printf("Step 5: zeroing pd_s\n");
        for (uint64_t i = 0; i < pdNum; ++i) {
            memset(pd_s + i * 512, 0, PAGE_SIZE);
        }
    
        m_window->printf("Step 6: wiring pml4\n");
        pml4[0] = reinterpret_cast<uint64_t>(pdpt) | 0x3;

        m_window->printf("Step 7: wiring pdpt\n");
        for (uint64_t i = 0; i < pdNum; i++)
            pdpt[i] = reinterpret_cast<uint64_t>(pd_s + i * 512) | 0x3;

        m_window->printf("Step 8: wiring pd_s\n");
        for (uint64_t i = 0; i < pdNum; i++)
            for (uint64_t j = 0; j < 512; j++)
                pd_s[i * 512 + j] = ((i * 0x40000000ULL) + (j * 0x200000ULL)) | 0x83;
    
        m_window->printf("Step 9: loading CR3\n");
        asm volatile("mov %0, %%cr3" :: "r"(pml4) : "memory");

        m_window->printf("Step 10: done\n");

        m_totalPages = highestAddress / static_cast<uint64_t>(PAGE_SIZE);
    }

    void PMM::ResetBitMap() {
        m_bitMapSize = (m_totalPages + 7) / 8; // 8 pages per bytes
        
        m_window->printf("Total pages=", m_totalPages, " BitMapSize=", m_bitMapSize, " bytes\n");

        // Finding usable region for BitMap
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;

            uint64_t regionEnd = entry.base + entry.length;
            uint64_t candidate = max(entry.base, KERNEL_END_ADDRESS, PageTables_End_Address);
            if (candidate >= regionEnd) continue;     // clamp pushed candidate past this region entirely
            
            uint64_t available = regionEnd - candidate;
            if (available >= m_bitMapSize) {
                m_bitMap = reinterpret_cast<uint8_t*>(candidate);
                break;
            }
        }

        KERNEL_ASSERT(m_bitMap != nullptr);

        m_window->printf("bitmap="); m_window->printfHex(reinterpret_cast<uint64_t>(m_bitMap));
        m_window->printf(" end=");   m_window->printfHex(reinterpret_cast<uint64_t>(m_bitMap) + m_bitMapSize);
        m_window->printf("\n");

        memset(m_bitMap, 0xFF, m_bitMapSize); // set all entries as used

        // free pages
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];

            if (entry.type != EntryType::Usable) continue;

            MarkRangeFree(entry.base, entry.length);
        }

        // mark reserved pages
        MarkRangeUsed(0, 0x100000);                                                                                 // first 1MB
        MarkRangeUsed(0x100000, reinterpret_cast<uint64_t>(_kernel_end));                                           // kernel & stack
        MarkRangeUsed(reinterpret_cast<uint64_t>(_kernel_end), PageTables_End_Address);                               // page tables
        MarkRangeUsed(reinterpret_cast<uint64_t>(m_bitMap), reinterpret_cast<uint64_t>(m_bitMap) + m_bitMapSize);   // pages BitMap
    }
    
    PMM::PageIndex PMM::getPageIndex(uint64_t page) const {
        return { .byte = page / 8, .bit = page % 8 };
    }
            
    inline void PMM::SetBit(uint64_t page) {
        if (page >= m_totalPages) return;
        const PageIndex index = getPageIndex(page);
        m_bitMap[index.byte] |= (1 << index.bit);
    }

    inline void PMM::ClearBit(uint64_t page) {
        if (page >= m_totalPages) return;
        const PageIndex index = getPageIndex(page);
        m_bitMap[index.byte] &= ~(1 << index.bit);
    }

    inline bool PMM::TestBit(uint64_t page) const {
        if (page >= m_totalPages) return false;
        const PageIndex index = getPageIndex(page);
        return m_bitMap[index.byte] & (1 << index.bit);
    }

    void PMM::MarkRangeUsed(uint64_t base, uint64_t end) {
        uint64_t startPage = base / PAGE_SIZE;
        uint64_t endPage = end / PAGE_SIZE;
        for (uint64_t page = startPage; page < endPage; ++page) {
            SetBit(page);
        }
    }

    void PMM::MarkRangeFree(uint64_t base, uint64_t length) {
        uint64_t startPage = base / PAGE_SIZE;
        uint64_t endPage = (base + length) / PAGE_SIZE;
        for (uint64_t page = startPage; page < endPage; ++page) {
            ClearBit(page);
        }
    }
}
