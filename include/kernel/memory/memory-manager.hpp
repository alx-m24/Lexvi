#pragma once

#include "internals/pmm.hpp"
#include "internals/vmm.hpp"

namespace kernel {
    class MemoryManager {
        public:
            PMM m_pmm{};
            VMM m_vmm{};

        public:
            MemoryManager() = default;

#ifndef BOOTLOADER
            void Init();
            void TestMemory();
#else
            void Init(Bytes kernelSize, Bytes ImageBase, Bytes ImageSize);
#endif
    };
}
