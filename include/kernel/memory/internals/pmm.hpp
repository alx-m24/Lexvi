#pragma once

#include "kernel/memory/memory-unit.hpp"
#include <cstdint>

namespace kernel {
    struct AllocationHeader {
        uint64_t pages = 1;
    };
    static constexpr uint64_t ALLOCATION_HEADER_SIZE = sizeof(AllocationHeader);

    // Class that handles physical pages, allocations and free's
    class PMM {
        private:
            static constexpr KiB PAGE_SIZE = 4_KiB;

        private:
            Bytes m_totalMemory{};
            uint64_t m_totalPageNum{};

            uint8_t* m_bitMap = nullptr;
            uint64_t m_bitMapSize{};

        public:
            PMM() = default;

        public:
            void Init();

            Bytes Alloc(Bytes bytes = PAGE_SIZE.bytes());
            void Free(Bytes address);

        private:
            void getTotalPageNum();

            void InitBitMap(Bytes reservedEnd);

            void CleanBitMap();

            void ZeroBitMap();
            void MarkUsedPages();

        private:
            void MarkRangeFree(Bytes base, Bytes length);
            void MarkRangeUsed(Bytes base, Bytes end);

        private:
            struct Page {
                uint64_t byte{};
                uint64_t bit{};
            };
            static constexpr Page getPage(uint64_t pageIdx) {
                return Page {
                    .byte = pageIdx / 8,
                    .bit = pageIdx % 8
                };
            };
            void SetBit(uint64_t page);
            void ClearBit(uint64_t page);
            bool TestBit(uint64_t page) const;
    };
}
