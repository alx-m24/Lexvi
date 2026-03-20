; src/kernel/entry.asm — linked with kernel-main binary
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
    mov rsp, stack_top
    xor rbp, rbp

    ; zero BSS
    mov rdi, _bss_start
    mov rcx, _bss_end
    sub rcx, rdi
    xor al, al
    rep stosb

    ; call global constructors
    mov rbx, _init_array_start
.ctor_loop:
    cmp rbx, _init_array_end
    jge .ctor_done
    mov rax, [rbx]
    call rax
    add rbx, 8
    jmp .ctor_loop
.ctor_done:

    call kernel_main_cpp
.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits
