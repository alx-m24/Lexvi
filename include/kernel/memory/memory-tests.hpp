// kernel/tests/memory-tests.hpp
#pragma once
#include "kernel/memory/memory-defs.hpp"
#include "kernel/memory/internals/pmm.hpp"
#include "kernel/memory/internals/vmm.hpp"
#include "kernel/console/console.hpp"
#include "kernel/error/error.hpp"

namespace kernel::tests {
#ifndef BOOTLOADER
// ─── PMM tests ──────────────────────────────────────────────────────────────

static void test_pmm(PMM& pmm) {
    kernel::printf("[TEST] PMM\n");

    // 1. single page alloc returns non-null
    void* p1 = pmm.Alloc(1);
    KERNEL_ASSERT(p1 != nullptr);

    // 2. returned address is in HHDM range
    KERNEL_ASSERT(reinterpret_cast<uint64_t>(p1) >= HHDM_BASE);

    // 3. two consecutive allocs return different pages
    void* p2 = pmm.Alloc(1);
    KERNEL_ASSERT(p1 != p2);

    // 4. multi-page alloc doesn't overlap with next alloc
    void* p3 = pmm.Alloc(4);
    KERNEL_ASSERT(p3 != nullptr);
    void* p4 = pmm.Alloc(1);
    uint64_t gap = reinterpret_cast<uint64_t>(p4) - reinterpret_cast<uint64_t>(p3);
    KERNEL_ASSERT(gap >= 4 * 4096);

    // 5. freed page is re-used by next alloc
    pmm.Free(p1);
    void* p5 = pmm.Alloc(1);
    KERNEL_ASSERT(p5 == p1);

    // 6. allocated page is writable
    volatile uint64_t* probe = reinterpret_cast<uint64_t*>(p2);
    *probe = 0xDEADBEEFCAFEBABEULL;
    KERNEL_ASSERT(*probe == 0xDEADBEEFCAFEBABEULL);

    // 7. physical page 0 is never handed out
    void* ptrs[256];
    bool zeroFound = false;
    for (int i = 0; i < 256; ++i) {
        ptrs[i] = pmm.Alloc(1);
        if (reinterpret_cast<uint64_t>(ptrs[i]) == HHDM_BASE)
            zeroFound = true;
    }
    KERNEL_ASSERT(!zeroFound);
    for (int i = 0; i < 256; ++i) pmm.Free(ptrs[i]);

    pmm.Free(p2);
    pmm.Free(p3);
    pmm.Free(p4);
    pmm.Free(p5);

    kernel::printf("[TEST] PMM passed\n");
}

// ─── VMM tests ──────────────────────────────────────────────────────────────

static void test_vmm(VMM& vmm, PMM& pmm) {
    kernel::printf("[TEST] VMM\n");

    constexpr uint64_t TEST_VIRT = 0xFFFF900000000000ULL;

    // 1. map a page and write to it
    void* phys = pmm.Alloc(1);
    KERNEL_ASSERT(phys != nullptr);

    uint64_t physAddr = TO_PHYS(reinterpret_cast<uint64_t>(phys));
    vmm.map(TEST_VIRT, physAddr, { .writable = true });

    volatile uint64_t* vptr = reinterpret_cast<uint64_t*>(TEST_VIRT);
    *vptr = 0x1234567890ABCDEFULL;
    KERNEL_ASSERT(*vptr == 0x1234567890ABCDEFULL);

    // 2. HHDM alias of the same physical page sees the same data
    volatile uint64_t* hhdm = reinterpret_cast<uint64_t*>(TO_VIRT(physAddr));
    KERNEL_ASSERT(*hhdm == 0x1234567890ABCDEFULL);

    // 3. write via HHDM is visible at the mapped virtual address
    *hhdm = 0xCAFEBABEDEADBEEFULL;
    KERNEL_ASSERT(*vptr == 0xCAFEBABEDEADBEEFULL);

    // 4. remap to a different physical page — data changes accordingly
    void* phys2 = pmm.Alloc(1);
    uint64_t physAddr2 = TO_PHYS(reinterpret_cast<uint64_t>(phys2));
    volatile uint64_t* hhdm2 = reinterpret_cast<uint64_t*>(TO_VIRT(physAddr2));
    *hhdm2 = 0xABABABABABABABABULL;

    vmm.unmap(TEST_VIRT);
    vmm.map(TEST_VIRT, physAddr2, { .writable = true });
    KERNEL_ASSERT(*vptr == 0xABABABABABABABABULL);

    // 5. kernel image is still accessible after all remapping
    volatile uint8_t* ktext = reinterpret_cast<uint8_t*>(KERNEL_VIRT_BASE);
    (void)*ktext;
    KERNEL_ASSERT(true);

    vmm.unmap(TEST_VIRT);
    pmm.Free(phys);
    pmm.Free(phys2);

    kernel::printf("[TEST] VMM passed\n");
}

// ─── entry point ────────────────────────────────────────────────────────────

static void RunAll(PMM& pmm, VMM& vmm) {
    kernel::printf("\n========== MEMORY TESTS ==========\n");
    test_pmm(pmm);
    test_vmm(vmm, pmm);
    kernel::printf("========== ALL PASSED ==========\n\n");
}
#endif
} // namespace kernel::tests
