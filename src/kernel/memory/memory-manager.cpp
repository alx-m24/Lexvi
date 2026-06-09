#include "kernel/memory/memory-manager.hpp"

#include "kernel/memory/memory-tests.hpp"
#include "kernel/memory/memory-defs.hpp"

using namespace kernel;

void MemoryManager::Init() {
    m_pmm.Init();
#ifndef BOOTLOADER
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    kernel::PageTable* currentPML4 = reinterpret_cast<kernel::PageTable*>(TO_VIRT(cr3));
    
    m_vmm.Init(currentPML4, m_pmm);
#else
    m_vmm.Init(m_pmm);
#endif
}

void MemoryManager::TestMemory() {
#ifndef BOOTLOADER
    tests::RunAll(m_pmm, m_vmm);
#endif
}
