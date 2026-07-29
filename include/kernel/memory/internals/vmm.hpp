#pragma once

#include "kernel/memory/internals/pmm.hpp"

namespace kernel {
    struct PageFlags {
        bool writable{};
        bool user{};
        bool noExecute{};
        bool hugePage{};
        bool cacheDisable{};  // PCD — bit 4
        uint64_t operator()() const {
            uint64_t flags = (1 << 0);
            if (writable)     flags |= (1 << 1);
            if (user)         flags |= (1 << 2);
            if (cacheDisable) flags |= (1 << 4);
            if (hugePage)     flags |= (1 << 7); // PS bit
            if (noExecute)    flags |= (1ULL << 63);
            return flags;
        }
    };    

    class PageTable;

    class PageTableEntry {
        // 63      NX  — No Execute
        // 62-52   (ignored)
        // 51-12   Physical address of next table (or page)
        // 11-9    (ignored/available)
        // 8       Global
        // 7       PS  — Page Size (in PD: 1=2MiB page, skip PT level)
        // 6       Dirty
        // 5       Accessed
        // 4       PCD — Cache Disable
        // 3       PWT — Write Through
        // 2       U/S — User/Supervisor
        // 1       R/W — Read/Write
        // 0       P   — Present
        uint64_t m_raw{};

        public:
            PageTableEntry() = default;
            PageTableEntry(uint64_t raw) : m_raw(raw) {}

        public:
            PageTable* getNextPageTable() const;

            bool isPresent() const;
            bool isHugePage() const;

            void set(uint64_t physAddr, PageFlags flags);
            void clear();
            
            uint64_t getPhys() const;
    };

    class PageTable {
        public:
            PageTableEntry entries[512];
        
            void clear();
    };

    class VMM {
        private:
            PMM* m_pmm = nullptr;

        private:
            PageTable* m_pml4;
            uint64_t   m_pml4Phys = 0;

        public:
            VMM() = default;

#ifndef BOOTLOADER
            void Init(PageTable* existingPML4, PMM& pmm);
#else
            void Init(PMM& pmm, Bytes ImageBase, Bytes ImageSize);
#endif

        public:
            void map(uint64_t virt, uint64_t phys, PageFlags flags);
            void mapMMIO(uint64_t virtBase, uint64_t physBase, Bytes size);
            void* Alloc(uint64_t virt, uint64_t pageCount, PageFlags flags);
            void unmap(uint64_t virt);
            void loadCR3();
            uint64_t getPML4Phys() const { return m_pml4Phys; }

    };
}
