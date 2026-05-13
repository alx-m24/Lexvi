#include "kernel/memory/internals/vmm.hpp"

namespace kernel {
    void VMM::Init(PMM& pmm) {
        m_pmm = &pmm;    
    }
    
    void VMM::Init(Bytes existingPML4, PMM& pmm) {
        m_pmm = &pmm;
    }
}
