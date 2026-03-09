#pragma once

#include <stdint.h>

#define IDT_TRAP_GATE      0x8F   // exceptions — doesn't disable interrupts
#define IDT_INTERRUPT_GATE 0x8E   // hardware IRQs — disables interrupts while handling
#define IDT_TRAP_GATE_USER 0xEF  // 0xEF = present, DPL=3, trap gate

struct idt_entry_t {
    uint16_t offset_low;    // handler address bits 0-15
    uint16_t selector;      // GDT kernel code selector
    uint8_t  ist;           // interrupt stack table, set 0 for now
    uint8_t  type_attr;     // type + privilege flags
    uint16_t offset_mid;    // handler address bits 16-31
    uint32_t offset_high;   // handler address bits 32-63
    uint32_t zero;          // reserved
} __attribute__((packed));

struct idtr_t {
    uint16_t limit;   // size of IDT - 1
    uint64_t base;    // address of IDT
} __attribute__((packed));

extern idt_entry_t idt[256];

void idt_init(void);
