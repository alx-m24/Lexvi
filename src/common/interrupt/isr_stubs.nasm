bits 64
section .text

extern isr_handler

%macro ISR_NOERR 1
    global isr_stub_%1
    isr_stub_%1:
        push 0
        push %1
        jmp isr_common
%endmacro

%macro ISR_ERR 1
    global isr_stub_%1
    isr_stub_%1:
        push %1
        jmp isr_common
%endmacro

%macro IRQ 2
    global irq_stub_%1
    irq_stub_%1:
        push 0
        push %2
        jmp isr_common
%endmacro

; No error code
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_NOERR 9
ISR_NOERR 16
ISR_NOERR 18
ISR_NOERR 19

; Has error code
ISR_ERR 8
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_ERR 17

; IRQ 0–15 mapped to vectors 32–47
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

global isr_stub_128
isr_stub_128:
    push 0      ; no error code
    push 128    ; vector 0x80
    jmp sys_common

isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call isr_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

sys_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call isr_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx

    add rsp, 8  ; skip rax
    add rsp, 16
    iretq 

section .note.GNU-stack noalloc noexec nowrite progbits
