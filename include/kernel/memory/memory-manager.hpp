#pragma once

#include "internals/pmm.hpp"
#include "internals/vmm.hpp"

namespace kernel {
    class MemoryManager {
        private:
            PMM m_pmm{};
            VMM m_vmm{};

        public:
            MemoryManager() = default;

            void Init();

        public:
            void TestMemory();
    };
}
