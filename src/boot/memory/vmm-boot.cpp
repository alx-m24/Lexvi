#include "common/memory/vmm.hpp"

#include "boot/memory/alloc.hpp"
#include "boot/bootContext.hpp"

#include "kernel/header.hpp"

#include "common/memory/memory-map.hpp"
#include "common/memory/pmm.hpp"
#include "common/memory/memory-defs.hpp"
#include "common/memory/address.hpp"
#include "common/utils/memory.hpp"
#include "common/error/assert.hpp"
#include "common/output/output-defs.hpp"

namespace Lexvi::Memory { 
    void VMM::Init(PMM& pmm, Bytes kernelSize, Bytes ImageBase, Bytes ImageSize) {
        m_pmm = &pmm;    
        // 1 page == 4_kb which equal sizeof(PageTable)
        m_pml4Phys = Boot::Memory::AllocPersistent<PageTable>(); 
        m_pml4 = reinterpret_cast<PageTable*>(m_pml4Phys.getValue()); // 1:1 mapping still active
        m_pml4->clear();

        // Kernel image (slot 511)
        uint64_t kernelPhys = Kernel::KERNEL_MAIN_LOAD_ADDR.getValue();  // 0x100000
        uint64_t kernel_Size = alignUp(kernelSize.count(), PMM::PAGE_SIZE.bytes().count());

        for (uint64_t off = 0; off < kernel_Size; off += PMM::PAGE_SIZE.bytes().count()) {
            map(KERNEL_VIRT_BASE.bytes().count() + off, kernelPhys + off, { .writable = true });
        }

        for (uint32_t i = 0; i < MEMORY_MAP_ENTRY_COUNT; ++i) {
            const E820Entry entry = E820Entries[i];

            uint64_t base = alignDown(entry.base, MiB(2).bytes().count());
            uint64_t end  = alignUp(entry.base + entry.length, MiB(2).bytes().count());

            for (uint64_t phys = base; phys < end; phys += MiB(2).bytes().count()) {
                map(
                    HHDM_BASE + phys,
                    phys,
                    { 
                        .writable = true,
                        .hugePage = true,
                        .cacheDisable = entry.type == EntryType::Usable ? false : true
                    });
                if (phys <= 0x100000) continue; // prevents double mapping of lower 1MB using 2 different page sizes
                if (entry.type == EntryType::Usable) map(
                    phys,
                    phys,
                    { 
                        .writable = true,
                        .hugePage = true,
                        .cacheDisable = entry.type == EntryType::Usable ? false : true
                    });
            }
        }

}
