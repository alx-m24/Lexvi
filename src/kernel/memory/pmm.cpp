#include "kernel/memory/internals/pmm.hpp"

#include "kernel/utils/math.hpp"
#include "kernel/utils/memory.hpp"
#include "kernel/memory/memory-unit.hpp"
#include "kernel/memory/internals/memory-map.hpp"

#ifndef BOOTLOADER
#include "kernel/console/console.hpp"
#define SAFE_PRINT(...) kernel::printf(__VA_ARGS__)
#else
void kernel_printf(char c);
void kernel_printf(const char* str);
void kernel_printf(uint32_t n);
void kernel_printf(uint64_t n);
void kernel_printf(double d);
void kernel_printf(uint16_t n);
void kernel_printf(int n);
void kernel_printfHex(uint64_t n);
void kernel_printfHex(uint32_t n);
void kernel_clearConsole();
template<typename First, typename... Others>
void kernel_printf(const First& first, const Others&... others) {
    kernel_printf(first);
    kernel_printf(others...);
}

#define SAFE_PRINT(...) kernel_printf(__VA_ARGS__)
#endif

namespace kernel {
    void PMM::Init() {
        SAFE_PRINT("[PMM] Initializing\n");
        getTotalPageNum();
        SAFE_PRINT("[PMM] Page Num: ", m_totalPageNum, '\n');

#ifndef BOOTLOADER
        InitBitMap(Bytes(KERNEL_END_ADDRESS));
#else
        InitBitMap(Bytes(0x100000 + KERNEL_MAIN_SECTORS * 512));
#endif


        CleanBitMap();

        SAFE_PRINT("[PMM] Successfully initialized\n");
    }

    void PMM::getTotalPageNum() {
        uint32_t* raw = reinterpret_cast<uint32_t*>(MEMORY_MAP_ENTRY_COUNT_ADDRESS);
        SAFE_PRINT("[PMM] Raw read in PMM: ", *raw, "\n");
        SAFE_PRINT("[PMM] MEMORY_MAP_ENTRY_COUNT in PMM: ", MEMORY_MAP_ENTRY_COUNT, "\n");

        Bytes highestAddress = 0_B;
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;

            Bytes end = Bytes(entry.base + entry.length);
            highestAddress = max(end, highestAddress);
        }

        m_totalMemory = highestAddress;

        SAFE_PRINT("[PMM] m_totalMemory bytes: ", m_totalMemory.count(), "\n");
        SAFE_PRINT("[PMM] PAGE_SIZE bytes: ", PAGE_SIZE.bytes().count(), "\n");
        SAFE_PRINT("[PMM] aligned: ", m_totalMemory.align_up(PAGE_SIZE).count(), "\n");

        m_totalPageNum = m_totalMemory.align_up(PAGE_SIZE) / PAGE_SIZE;
    }

    void PMM::InitBitMap(Bytes reservedEnd) {
        m_bitMapSize = (m_totalPageNum + 7) / 8;

        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];
            if (entry.type != EntryType::Usable) continue;

            uint64_t regionEnd = entry.base + entry.length;
            uint64_t candidate = max(entry.base, reservedEnd.count());
            if (candidate >= regionEnd) continue;     // clamp pushed candidate past this region entirely
            
            uint64_t available = regionEnd - candidate;
            if (available >= m_bitMapSize) {
                m_bitMap = reinterpret_cast<uint8_t*>(candidate);
                break;
            }
        }
    }

    void PMM::CleanBitMap() {
        ZeroBitMap();
        MarkUsedPages();
    }

    void PMM::ZeroBitMap() {
        SAFE_PRINT("[PMM] m_bitMap: ", reinterpret_cast<uint64_t>(m_bitMap), "\n");
        memset(m_bitMap, 0xFF, m_bitMapSize); // set all entries as used

        // free pages
        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];

            if (entry.type != EntryType::Usable) continue;

            MarkRangeFree(Bytes(entry.base), Bytes(entry.length));
        }
    }

    void PMM::MarkUsedPages() {
        MarkRangeUsed(0_B, 0x100000_B);  // first 1MB - same in both contexts

#ifndef BOOTLOADER
        // kernel-main: mark from 1MB to kernel_end (includes kernel image + stack)
        MarkRangeUsed(0x100000_B, Bytes(reinterpret_cast<uint64_t>(_kernel_end)));
#else
        // stage 2: mark temp load area and final kernel destination conservatively
        MarkRangeUsed(Bytes(TEMP_KERNEL_MAIN_LOAD_ADDR), 
                      Bytes(TEMP_KERNEL_MAIN_LOAD_ADDR + KERNEL_MAIN_SECTORS * 512));
        MarkRangeUsed(0x100000_B, Bytes(0x100000 + KERNEL_MAIN_SECTORS * 512));
#endif

        // bitmap itself - same in both contexts
        MarkRangeUsed(Bytes(reinterpret_cast<uint64_t>(m_bitMap)), 
        Bytes(reinterpret_cast<uint64_t>(m_bitMap) + m_bitMapSize));
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


    Bytes PMM::Alloc(Bytes bytes) {
        const uint64_t pagesNeeded = (bytes + Bytes(ALLOCATION_HEADER_SIZE)).align_up(PAGE_SIZE) / PAGE_SIZE;
        for (uint64_t i = 0; i < m_totalPageNum; ++i) {
            bool found = true;
            for (uint64_t j = 0; j < pagesNeeded && found; ++j) {
                found = !TestBit(i + j); // TestBit() return TRUE if used -> FOUND should be true when EMPTY
            }

            if (found) {
                const Bytes startPageAddress = PAGE_SIZE.bytes() * i;
                MarkRangeUsed(startPageAddress, startPageAddress + (PAGE_SIZE.bytes() * pagesNeeded));
                AllocationHeader* headerPtr = reinterpret_cast<AllocationHeader*>(startPageAddress.count());
                headerPtr->pages = pagesNeeded;
                return startPageAddress + Bytes(ALLOCATION_HEADER_SIZE);
            }
        }
        return Bytes(0);
    }

    void PMM::Free(Bytes address) {
        AllocationHeader* header = reinterpret_cast<AllocationHeader*>(address.count()) - 1;
        const Bytes startAddress = Bytes(reinterpret_cast<uint64_t>(header));
        
        MarkRangeFree(startAddress, PAGE_SIZE.bytes() * header->pages);
    }
}
