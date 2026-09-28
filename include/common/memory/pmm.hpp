#pragma once

#include "common/types/result.hpp"
#include "common/memory/address.hpp"
#include "common/memory/memory-unit.hpp"

namespace Lexvi::Memory {
    // Construct's bitmap
    // Mirrors Allocations that outlive efi bootloader in bitmap
    class PMM {
        private: 
            Memory::PhysicalAddress m_bitMap{};
            uint64_t m_bitMapSize{};
            uint64_t m_TotalPageNum{};

        public:
            static inline constexpr Memory::Bytes PAGE_SIZE = 4_KiB;

        public:
            PMM() = default;

        public:
            Types::Result Init(); 
            void Shutdown();

        private:
            Types::Result SyncBitMap();

        private:
            void MarkRangeFree(Memory::Bytes base, Memory::Bytes length);
            void MarkRangeUsed(Memory::Bytes base, Memory::Bytes end);

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

        public:
            Memory::PhysicalAddress getBitMapPhys() const;
    };

}
