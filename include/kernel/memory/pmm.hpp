#pragma once

#include "kernel/kernel-config.hpp"

#include <stdint.h>

namespace kernel {
   class MemoryWindow;
    
    enum class EntryType : uint32_t {
        Usable = 1,
        Reserved = 2,
        ACPI_Reclaimable = 3,
        ACPI_NVS = 4,
        Bad_Memory = 5
    };

    struct E820Entry {
        uint64_t base;
        uint64_t length;
        EntryType type;
        uint32_t acpi_attrs;
    } __attribute__((packed));

    class PMM {
        private:
            MemoryWindow* m_window = nullptr;

        private:
            static constexpr uint32_t PAGE_SIZE = 4096; // 4kb
            static constexpr uint64_t pdEntriesNum = 512;

            const uint64_t KERNEL_END_ADDRESS = reinterpret_cast<uint64_t>(_kernel_end);
            const uint32_t MEMORY_MAP_ENTRY_COUNT = *reinterpret_cast<uint32_t*>(MEMORY_MAP_ENTRY_COUNT_ADDRESS);
            const E820Entry* const E820Entries = reinterpret_cast<E820Entry*>(MEMORY_MAP_ADDRESS);

            uint64_t m_totalPages = 0;

            uint64_t* pml4 = reinterpret_cast<uint64_t*>(KERNEL_END_ADDRESS);
            uint64_t* pdpt = pml4 + 512;    // 4KB after PML4   // covers 512 GB
            uint64_t* pd_s = 0;     // 4KB after previous and so on... // covers 1 GB each

            uint64_t pdNum = 1;

            uint64_t PageTables_End_Address = 0;

            uint8_t* m_bitMap = nullptr;
            uint64_t m_bitMapSize = 0;

        public:
            PMM() = default;
    
        public:
            void Init(MemoryWindow* window);

        private:
            void ResetPageTable();
            void ResetBitMap();

        private:
            struct PageIndex {
                uint64_t byte = 0;
                uint64_t bit = 0;
            };
            PageIndex getPageIndex(uint64_t page) const;

            inline void SetBit(uint64_t page);
            inline void ClearBit(uint64_t page);
            inline bool TestBit(uint64_t page) const;

        private:
            void MarkRangeUsed(uint64_t base, uint64_t end);
            void MarkRangeFree(uint64_t base, uint64_t length);
    };
}
