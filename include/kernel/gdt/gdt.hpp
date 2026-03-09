#pragma once
#include <stdint.h>


// Access bits
#define GDT_PRESENT   (1 << 7)
#define GDT_NOT_SYS   (1 << 4)
#define GDT_EXEC      (1 << 3)
#define GDT_DC        (1 << 2)
#define GDT_RW        (1 << 1)
#define GDT_ACCESSED  (1 << 0)

// Flag bits
#define GDT_GRAN_4K   (1 << 7)
#define GDT_SZ_32     (1 << 6)
#define GDT_LONG_MODE (1 << 5)

// Privilege levels
#define GDT_RING0     (0 << 5)
#define GDT_RING3     (3 << 5)

struct gdt_entry_t {
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags;      // high nibble = flags, low nibble = limit[16:19]
    uint8_t  base_hi;
}__attribute__((packed));

struct gdtr_t {
    uint16_t limit;      // size of GDT - 1
    uint64_t base;       // address of GDT
} __attribute__((packed));

// Selector offsets (byte index into GDT)
#define GDT_SEL_NULL  0x00
#define GDT_SEL_CODE  0x08
#define GDT_SEL_DATA  0x10
#define GDT_SEL_UDATA 0x18
#define GDT_SEL_UCODE 0x20

extern gdt_entry_t gdt[];

void gdt_load(void);
