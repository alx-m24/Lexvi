#include "kernel/gdt/gdt.hpp"

gdt_entry_t gdt[] = {
    // Null
    [0] = { 0 },

    // Kernel Code — Ring 0, 64-bit
    [1] = {
        .limit_lo = 0xFFFF,
        .base_lo  = 0,
        .base_mid = 0,
        .access   = GDT_PRESENT | GDT_NOT_SYS | GDT_EXEC | GDT_RW | GDT_RING0,
        .flags    = GDT_GRAN_4K | GDT_LONG_MODE | 0xF,
        .base_hi  = 0
    },

    // Kernel Data — Ring 0, 32-bit size flag
    [2] = {
        .limit_lo = 0xFFFF,
        .base_lo  = 0,
        .base_mid = 0,
        .access   = GDT_PRESENT | GDT_NOT_SYS | GDT_RW | GDT_RING0,
        .flags    = GDT_GRAN_4K | 0xF,
        .base_hi  = 0
    },

    // User Data — Ring 3
    [3] = {
        .limit_lo = 0xFFFF,
        .base_lo  = 0,
        .base_mid = 0,
        .access   = GDT_PRESENT | GDT_NOT_SYS | GDT_RW | GDT_RING3,
        .flags    = GDT_GRAN_4K | 0xF,
        .base_hi  = 0
    },

    // User Code — Ring 3, 64-bit
    [4] = {
        .limit_lo = 0xFFFF,
        .base_lo  = 0,
        .base_mid = 0,
        .access   = GDT_PRESENT | GDT_NOT_SYS | GDT_EXEC | GDT_RW | GDT_RING3,
        .flags    = GDT_GRAN_4K | GDT_LONG_MODE | 0xF,
        .base_hi  = 0
    },
};

void gdt_load(void) {
    gdtr_t gdtr = {
        .limit = sizeof(gdt) - 1,
        .base  = (uint64_t)&gdt
    };
    
    asm volatile (
        "lgdt %0\n"
        "pushq %1\n"                 // push kernel code selector
        "leaq 1f(%%rip), %%rax\n"    // AT&T: dst is last, rip-relative lea
        "pushq %%rax\n"
        "lretq\n"                    // AT&T name for far retq
        "1:\n"
        "movw %2, %%ax\n"            // AT&T: need size suffix + %% for registers
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
        :
        : "m"(gdtr),
          "i"(GDT_SEL_CODE),
          "i"(GDT_SEL_DATA)
        : "rax", "memory"
    );
}
