#include "kernel/memory/memory-manager.hpp"

#include "kernel/memory/memory-tests.hpp"
#include "kernel/memory/memory-defs.hpp"

using namespace kernel;

#ifndef BOOTLOADER
void MemoryManager::Init() {
    m_pmm.Init();

    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    kernel::PageTable* currentPML4 = reinterpret_cast<kernel::PageTable*>(TO_VIRT(cr3));
    
    m_vmm.Init(currentPML4, m_pmm);
}

void MemoryManager::TestMemory() {
    tests::RunAll(m_pmm, m_vmm);
}

#else
    void MemoryManager::Init(Bytes kernelSize, Bytes ImageBase, Bytes ImageSize) {
        m_pmm.Init(kernelSize, ImageBase, ImageSize);
        m_vmm.Init(m_pmm, kernelSize, ImageBase, ImageSize);
    }
#endif
