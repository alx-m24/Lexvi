#pragma once

#include "kernel/memory/internals/pmm.hpp"
#include "kernel/memory/memory-unit.hpp"

namespace kernel {
    struct PageFlags {
        bool writable{};
        bool user{};
        bool noExecute{};

        uint64_t operator()() const {
            uint64_t flags = (1 << 0);                        // Present bit, always set
            if (writable)  flags |= (1 << 1);                 // R/W
            if (user)      flags |= (1 << 2);                 // U/S
            if (noExecute) flags |= (1ULL << 63);             // NX — needs ULL, bit 63
            return flags;
        }
    };

    class VMM {
        private:
            PMM* m_pmm = nullptr;

        private:
            Bytes m_pml4Address;

        public:
            VMM() = default;

            void Init(PMM& pmm);
            void Init(Bytes existingPML4, PMM& pmm);
    };
}
