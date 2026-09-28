#include "common/interrupt/idt.hpp"
#include "common/interrupt/pic.hpp"
#include "common/interrupt/frame.hpp"

#include "common/asm.hpp"
#include "common/error/panic.hpp"
#include "common/output/output-defs.hpp"

#include "common/keyboard/keyboard.hpp"
#include "common/time/time.hpp"
#include "common/syscall/syscall.hpp"
#include "common/keyboard/ps2.hpp"
#include "common/gdt/gdt.hpp"

#include <cstdint>

extern "C" {
    void isr_stub_0();
    void isr_stub_1();
    void isr_stub_2();
    void isr_stub_3();
    void isr_stub_4();
    void isr_stub_5();
    void isr_stub_6();
    void isr_stub_7();
    void isr_stub_8();
    void isr_stub_9();
    void isr_stub_10();
    void isr_stub_11();
    void isr_stub_12();
    void isr_stub_13();
    void isr_stub_14();
    void isr_stub_16();
    void isr_stub_17();
    void isr_stub_18();
    void isr_stub_19();
    void irq_stub_0();
    void irq_stub_1();
    void irq_stub_2();
    void irq_stub_3();
    void irq_stub_4();
    void irq_stub_5();
    void irq_stub_6();
    void irq_stub_7();
    void irq_stub_8();
    void irq_stub_9();
    void irq_stub_10();
    void irq_stub_11();
    void irq_stub_12();
    void irq_stub_13();
    void irq_stub_14();
    void irq_stub_15();

    void isr_stub_128();
}

namespace Lexvi::Interrupt {
    idt_entry_t idt[256] {};
    
    static const char* exception_names[] = {
        "Division Error", "Debug", "Non-Maskable Interrupt", "Breakpoint",
        "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available",
        "Double Fault", "Reserved", "Invalid TSS", "Segment Not Present",
        "Stack Fault", "General Protection", "Page Fault", "Reserved",
        "x87 FPU Error", "Alignment Check", "Machine Check", "SIMD FP Exception",
    };
    
    enum class IRQ : uint8_t {
        SYSTEM_TIMER = 0,
        KEYBOARD = 1
    };
    
    extern "C" void isr_handler(Interrupt::Frame *frame) {
        if (frame->vector == 128) { // suscall
            uint64_t rax = handle_syscall(*frame);
            frame->rax = rax;
            return;
        }
        if (frame->vector >= 32) { // IRQ
            uint8_t irq = frame->vector - 32;
    
            switch (static_cast<IRQ>(irq)) {
                case IRQ::SYSTEM_TIMER: Time::timerTick(); break;
                case IRQ::KEYBOARD: Keyboard::HandleKeyBoardIRQ(); break;
                default: LEXVI_PRINT("Unknown IRQ: ", static_cast<uint32_t>(irq), '\n'); break;
            }
    
            pic_eoi(irq);
            return;
        }
    
        const char* name = (frame->vector < 20) ? exception_names[frame->vector] : "Unknown";
    
        LEXVI_PRINT("\n=== EXCEPTION ===\n");
        LEXVI_PRINT("Vector: "); LEXVI_PRINTHEX(frame->vector);
        LEXVI_PRINT(" ("); LEXVI_PRINT(name); LEXVI_PRINT(")\n");
        LEXVI_PRINT("Error:  "); LEXVI_PRINTHEX(frame->error_code); LEXVI_PRINT("\n");
        LEXVI_PRINT("RIP:    "); LEXVI_PRINTHEX(frame->rip);        LEXVI_PRINT("\n");
        LEXVI_PRINT("RSP:    "); LEXVI_PRINTHEX(frame->rsp);        LEXVI_PRINT("\n");
        LEXVI_PRINT("CS:     "); LEXVI_PRINTHEX(frame->cs);         LEXVI_PRINT("\n");
    
        if (frame->vector == 14) {
            uint64_t cr2;
            ASM("mov %%cr2, %0" : "=r"(cr2));
            LEXVI_PRINT("CR2:    "); LEXVI_PRINT(cr2); LEXVI_PRINT("\n");
        }
    
        LEXVI_PANIC("CPU Exception");
    }
    
    void idt_set(uint8_t vector, void *handler, uint8_t type_attr) {
        uint64_t addr = (uint64_t)handler;
        idt[vector].offset_low  = addr & 0xFFFF;
        idt[vector].offset_mid  = (addr >> 16) & 0xFFFF;
        idt[vector].offset_high = (addr >> 32) & 0xFFFFFFFF;
        idt[vector].selector    = GDT_SEL_CODE;
        idt[vector].ist         = 0;
        idt[vector].type_attr   = type_attr;
        idt[vector].zero        = 0;
    }
    
    void idt_load(void) {
        idtr_t idtr = {
            .limit = sizeof(idt) - 1,
            .base  = (uint64_t)&idt
        };
    
        ASM("lidt %0" : : "m"(idtr) : "memory");
    }
    
    Types::Result IDT::Init() {
        LEXVI_PRINT("Initializing IDT\n");

        ASM("cli");
    
        idt_set(0,  (void*)isr_stub_0,  IDT_TRAP_GATE);
        idt_set(1,  (void*)isr_stub_1,  IDT_TRAP_GATE);
        idt_set(2,  (void*)isr_stub_2,  IDT_TRAP_GATE);
        idt_set(3,  (void*)isr_stub_3,  IDT_TRAP_GATE);
        idt_set(4,  (void*)isr_stub_4,  IDT_TRAP_GATE);
        idt_set(5,  (void*)isr_stub_5,  IDT_TRAP_GATE);
        idt_set(6,  (void*)isr_stub_6,  IDT_TRAP_GATE);
        idt_set(7,  (void*)isr_stub_7,  IDT_TRAP_GATE);
        idt_set(8,  (void*)isr_stub_8,  IDT_TRAP_GATE);
        idt_set(9,  (void*)isr_stub_9,  IDT_TRAP_GATE);
        idt_set(10, (void*)isr_stub_10, IDT_TRAP_GATE);
        idt_set(11, (void*)isr_stub_11, IDT_TRAP_GATE);
        idt_set(12, (void*)isr_stub_12, IDT_TRAP_GATE);
        idt_set(13, (void*)isr_stub_13, IDT_TRAP_GATE);
        idt_set(14, (void*)isr_stub_14, IDT_TRAP_GATE);
        idt_set(16, (void*)isr_stub_16, IDT_TRAP_GATE);
        idt_set(17, (void*)isr_stub_17, IDT_TRAP_GATE);
        idt_set(18, (void*)isr_stub_18, IDT_TRAP_GATE);
        idt_set(19, (void*)isr_stub_19, IDT_TRAP_GATE);
    
        idt_set(32, (void*)irq_stub_0, IDT_INTERRUPT_GATE);
        idt_set(33, (void*)irq_stub_1, IDT_INTERRUPT_GATE);
        idt_set(34, (void*)irq_stub_2, IDT_INTERRUPT_GATE);
        idt_set(35, (void*)irq_stub_3, IDT_INTERRUPT_GATE);
        idt_set(36, (void*)irq_stub_4, IDT_INTERRUPT_GATE);
        idt_set(37, (void*)irq_stub_5, IDT_INTERRUPT_GATE);
        idt_set(38, (void*)irq_stub_6, IDT_INTERRUPT_GATE);
        idt_set(39, (void*)irq_stub_7, IDT_INTERRUPT_GATE);
        idt_set(40, (void*)irq_stub_8, IDT_INTERRUPT_GATE);
        idt_set(41, (void*)irq_stub_9, IDT_INTERRUPT_GATE);
        idt_set(42, (void*)irq_stub_10, IDT_INTERRUPT_GATE);
        idt_set(43, (void*)irq_stub_11, IDT_INTERRUPT_GATE);
        idt_set(44, (void*)irq_stub_12, IDT_INTERRUPT_GATE);
        idt_set(45, (void*)irq_stub_13, IDT_INTERRUPT_GATE);
        idt_set(46, (void*)irq_stub_14, IDT_INTERRUPT_GATE);
        idt_set(47, (void*)irq_stub_15, IDT_INTERRUPT_GATE);
    
        idt_set(128, (void*)isr_stub_128, IDT_TRAP_GATE_USER);
    
        idt_load();

        pic_remap(Interrupts{ .systemClock = true, .keyboard = true });
        pit_init(Time::CLOCK_FREQ);
        Keyboard::PS2::Init();
    
        ASM("sti");

        LEXVI_PRINT(" - Successfully initialized IDT\n");

        return {};
    }

    void IDT::Shutdown() { }
}
