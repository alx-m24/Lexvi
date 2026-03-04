org 0x8000
[bits 32]

VGA_MEMORY equ 0xB8000

stage2_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov ecx, 80 ; equivalent of newline
    mov bl, 0x0F
    mov esi, greetingMsg
    call print_string_vga

hang:
    jmp hang

print_char_vga:
    mov edi, VGA_MEMORY     ; load base into register
    mov edx, ecx
    shl edx, 1
    add edi, edx            ; edi = exact byte address in VGA buffer
    mov ah, bl
    mov [edi], ax           ; write char + color
    ret

print_string_vga:
.next_char:
    lodsb
    cmp al, 0
    je .done
    call print_char_vga
    inc ecx
    jmp .next_char
.done:
    ret

greetingMsg db "Welcome to Lexvi's stage 2. Successfully switched to Protected mode(32-bit)", 0
