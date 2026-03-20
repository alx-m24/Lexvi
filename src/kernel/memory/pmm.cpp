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

    void PMM::MapAllMemory_2MB() {
        uint64_t highestAddress = 0;
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;
            uint64_t end = entry.base + entry.length;
            if (end > highestAddress) highestAddress = end;
        }
    
        pdNum = (highestAddress + 0x3FFFFFFF) / 0x40000000;
    
        // Temp tables in already-mapped low memory
        uint64_t* temp_pml4 = reinterpret_cast<uint64_t*>(0x50000);
        uint64_t* temp_pdpt = reinterpret_cast<uint64_t*>(0x51000);
        // PDs start at 0x52000, one 4KB page each
        
        memset(temp_pml4, 0, TABLE_ENTRIES_NUM * 8);
        memset(temp_pdpt, 0, TABLE_ENTRIES_NUM * 8);
        temp_pml4[0] = reinterpret_cast<uint64_t>(temp_pdpt) | 0x3;
    
        for (uint64_t i = 0; i < pdNum; i++) {
            uint64_t* temp_pd = reinterpret_cast<uint64_t*>(0x52000 + i * 0x1000);
            memset(temp_pd, 0, TABLE_ENTRIES_NUM * 8);
            temp_pdpt[i] = reinterpret_cast<uint64_t>(temp_pd) | 0x3;
            for (uint64_t j = 0; j < 512; j++)
                temp_pd[j] = ((i * 0x40000000ULL) + (j * 0x200000ULL)) | 0x83;
        }
    
        asm volatile("mov %0, %%cr3" :: "r"(temp_pml4) : "memory");
        
        m_allMemoryMapped = true;
    }

    void PMM::ResetPageTable() {
        if (!m_allMemoryMapped) {
            kernel::printf("Mapping All system memory\n");
            MapAllMemory_2MB();
        }

        m_window->printf("Step 1: finding highest address\n");

        uint64_t highestAddress = 0;
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;
            uint64_t end = entry.base + entry.length;
            if (end > highestAddress) highestAddress = end;
        }
        m_window->printf("highestAddress="); m_window->printfHex(highestAddress); m_window->printf("\n");
    
        m_window->printf("Step 2: computing pdNum & ptNum\n");

        pd_s = pdpt + 512;
        pdNum = (highestAddress + 0x3FFFFFFF) / 0x40000000; // round up to nearest 1GB

        pt_s = pd_s + pdNum * TABLE_ENTRIES_NUM;
        ptNum = (highestAddress + 0x1FFFFF) / 0x200000;  // round up to nearest 2MB

        PageTables_End_Address = reinterpret_cast<uint64_t>(pt_s + ptNum * TABLE_ENTRIES_NUM);

        m_window->printf("pdNum=", pdNum, " ptNum=", ptNum, '\n');
        m_window->printf("pml4="); m_window->printfHex(reinterpret_cast<uint64_t>(pml4)); m_window->printf("\n");
        m_window->printf("pdpt="); m_window->printfHex(reinterpret_cast<uint64_t>(pdpt)); m_window->printf("\n");
        m_window->printf("pd_s="); m_window->printfHex(reinterpret_cast<uint64_t>(pd_s)); m_window->printf("\n");
        m_window->printf("pt_s="); m_window->printfHex(reinterpret_cast<uint64_t>(pt_s)); m_window->printf("\n");

        m_window->printf("Step 3: zeroing pml4\n");
        // Zero PML4 (512 entries × 8 bytes = 4KB)
        memset(pml4, 0, TABLE_ENTRIES_NUM * 8);
        
        m_window->printf("Step 4: zeroing pdpt\n");
        // Zero PDPT (512 entries × 8 bytes = 4KB)
        memset(pdpt, 0, TABLE_ENTRIES_NUM * 8);
        
        m_window->printf("Step 5: zeroing pd_s\n");
        // Zero each PD (pdNum PDs, each 512 entries × 8 bytes = 4KB)
        for (uint64_t i = 0; i < pdNum; ++i) {
            memset(pd_s + i * 512, 0, TABLE_ENTRIES_NUM * 8);
        }

        m_window->printf("Step 6: zeroing pt_s\n");
        // Zero each PT (ptNum PTs, each 512 entries × 8 bytes = 4KB)
        for (uint64_t i = 0; i < ptNum; ++i) {
            memset(pt_s + i * 512, 0, TABLE_ENTRIES_NUM * 8);
        }
        
        m_window->printf("Step 7: wiring pml4\n");
        pml4[0] = reinterpret_cast<uint64_t>(pdpt) | 0x3;

        m_window->printf("Step 8: wiring pdpt\n");
        for (uint64_t i = 0; i < pdNum; i++)
            pdpt[i] = reinterpret_cast<uint64_t>(pd_s + i * 512) | 0x3;

        m_window->printf("Step 9: wiring pd_s to pt_s\n");
        for (uint64_t i = 0; i < pdNum; i++) {
            for (uint64_t j = 0; j < 512; j++) {
                uint64_t ptIndex = i * 512 + j;
                if (ptIndex >= ptNum) break;
                pd_s[i * 512 + j] = reinterpret_cast<uint64_t>(pt_s + ptIndex * 512) | 0x3;
            }
        }
        
        m_window->printf("Step 10: wiring pt_s to physical pages");
        for (uint64_t i = 0; i < ptNum; i++) {
            for (uint64_t j = 0; j < 512; j++) {
                uint64_t physAddr = (i * 0x200000ULL) + (j * 0x1000ULL);
                pt_s[i * 512 + j] = physAddr | 0x3; // present | rw
            }
        }
        
        m_window->printf("Step 11: loading CR3\n");
        asm volatile("mov %0, %%cr3" :: "r"(pml4) : "memory");

        m_window->printf("Step 12: done\n");
        m_totalPages = highestAddress / static_cast<uint64_t>(PAGE_SIZE);
    }

    void PMM::ResetBitMap() {
        m_bitMapSize = (m_totalPages + 7) / 8; // 8 pages per bytes
        
        m_window->printf("Total pages=", m_totalPages, "\nBitMapSize=", m_bitMapSize, " bytes");

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
        MarkRangeUsed(reinterpret_cast<uint64_t>(_kernel_end), PageTables_End_Address);                             // page tables
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
