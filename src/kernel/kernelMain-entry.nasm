; src/kernel/kernelMain-entry.asm
[bits 64]

section .text.kernel_main
global kernel_main
extern kernel_main_cpp
extern stack_top
extern _init_array_start
extern _init_array_end
extern _bss_start
extern _bss_end

kernel_main:
    ; ── stack ────────────────────────────────────────────────────────────────
    lea rax, [rel stack_top]
    mov rsp, rax
    xor rbp, rbp

    ; ── zero BSS ─────────────────────────────────────────────────────────────
    lea rdi, [rel _bss_start]
    lea rcx, [rel _bss_end]
    sub rcx, rdi
    jz .bss_done          ; nothing to zero
    xor eax, eax
    rep stosb

.bss_done:

    ; ── call global constructors ──────────────────────────────────────────────
    lea rbx, [rel _init_array_start]
    lea r12, [rel _init_array_end]


.ctor_loop:
    cmp rbx, r12
    jae .ctor_done
    mov rax, [rbx]
    test rax, rax          ; skip null entries
    jz .ctor_next
    call rax
.ctor_next:
    add rbx, 8
    jmp .ctor_loop
.ctor_done:

    ; ── jump to C++ kernel ────────────────────────────────────────────────────
    call kernel_main_cpp

.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits
