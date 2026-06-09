#pragma once

#include "kernel/memory/memory-unit.hpp"
#include <cstdint>

namespace kernel {
    class PMM {
        public:

        private:
            Bytes m_totalMemory{};
            uint64_t m_totalPageNum{};

            uint8_t* m_bitMap = nullptr;
            uint64_t m_bitMapSize{};

        public:
            PMM() = default;

        public:
            void Init();

            void* Alloc(uint64_t pages);
            void Free(void* address);

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
        public:
            bool TestBit(uint64_t page) const;
    };
}
