#include "common/memory/pmm.hpp"

#include <cstdint>

using namespace Lexvi::Types;
using namespace Lexvi::Memory;

namespace Lexvi::Memory {
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
        if (page >= m_TotalPageNum) return;
        const Page index = getPage(page);
        reinterpret_cast<uint8_t*>(m_bitMap.getValue())[index.byte] |= (1 << index.bit);
    }

    void PMM::ClearBit(uint64_t page) {
        if (page >= m_TotalPageNum) return;
        const Page index = getPage(page);
        reinterpret_cast<uint8_t*>(m_bitMap.getValue())[index.byte] &= ~(1 << index.bit);
    }

    bool PMM::TestBit(uint64_t page) const {
        if (page >= m_TotalPageNum) return true; // out of bounds = treat as used
        const Page index = getPage(page);
        return reinterpret_cast<uint8_t*>(m_bitMap.getValue())[index.byte] & (1 << index.bit);
    }

    PhysicalAddress PMM::getBitMapPhys() const {
        return m_bitMap;
    }
}
