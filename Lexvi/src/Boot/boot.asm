[org 0x7C00]       ; BIOS loads boot sector here
mov ah, 0x0E       ; BIOS teletype output

mov si, msg      ; point to the message
print_loop:
    lodsb           ; load byte at DS:SI into AL and increment SI
    cmp al, 0      ; check for null terminator
    je done         ; if null, we are done
    int 0x10       ; call BIOS to print character in AL
    jmp print_loop  ; repeat for next character
done:
    mov ax, 0x0000  ; set up infinite loop
    ; setup protected mode
    call enable_protected_mode
    ; jump to C++ kernel start address (let's say 0x100000)
    jmp 0x08:0x100000

msg db 'Hello, from bootloader!', 0

times 510-($-$$) db 0  ; fill the rest of sector
dw 0xAA55              ; boot signature
