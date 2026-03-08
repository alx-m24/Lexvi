%include "asm/boot-config.nasm"

org 0x7C00
[bits 16]

; This runs right after BIOS/Start-Up in 16-bit real mode

; We have 16-bit registers AX, BX, CX and DX
; The prefix H and L tells us to write to the upper or lower 8-bits respectively

; INT instruction -> INTERRUPT
_start:
    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00    ; stack grows down from bootloader start

    ; Setting cursor mode;
    ;   - AH -> bios function
    ;   - CH -> cursor blink(bits 5 & 6) and top most scan lines (bits 0 - 4)
    ;   - CL -> lower most scan lines (bits 0 - 4)
    ; Note: CH/CL values are scan line indexes, not pixels directly. Therefore scan line bits determine the height of the cursor
    mov ah, 01h
    mov ch, 0b00011111
    mov cl, 0b00011111
    int 10h ; call interrupt 10h
    
    ; Setting cursor position
    mov ah, 02h
    mov bh, 00h ; page number & graphics mode
    mov dh, 00h ; row (00h is top)
    mov dl, 00h ; column (00h is left)
    int 10h

    mov si, greetingMsg
    call printString

    ; loading stage-2 at absolute address 0x8000
    ; setting up buffer
    ; loading stage-2 at physical address 0x8000
    xor ax, ax
    mov es, ax       ; ES = 0x0000
    mov bx, 0x8000   ; BX = 0x8000 -> :BX = 0x0000:0x8000 = phys 0x8000
    
    mov ah, 02h
    mov al, STAGE2_SECTORS
    mov ch, 0 ; cylinder 0
    mov cl, 2 ; sector number
    mov dh, 0 ; head 0
    mov dl, [boot_drive]
    int 13h
    
    jc disk_error ; error -> carry flag set

    ; switching to protected mode (32-bit)
    cli ; disable 16-bit real mode interrupts
    call enable_a20 
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1 ; setting protection enabled
    mov cr0, eax

    jmp KernelCodeSelector:protected_mode_entry


disk_error:
    mov si, diskErrorMsg
    call printString
    jmp $ ; jump to current address -> infinite loop

enable_a20:
    in   al, 0x92
    or   al, 00000010b   ; Set bit 1 (A20 enable)
    out  0x92, al
    ret

printChar:
    mov ah, 0Eh ; teletype output (prints character and advances cursor)
    mov bh, 0 ; page number
    int 10h
    ret

printString:
    lodsb ; loads and increments si
    cmp al, 0 ; compares loaded value to null termination
    je donePrinting
    call printChar
    jmp printString
donePrinting:
    ret

[bits 32]
protected_mode_entry:
    mov ax, KernelDataSelector  ; kernel data

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000 ; safe stack

    ; far jump to flush prefetch and load CS
    ; Jump to stage-2 at physical address 0x8000
    jmp 0x8000

[bits 16]
gdt_start:
; Bits:
;     0-15   = limit low 
;     16-31  = base low
;     32-39  = base middle
;     40-47  = access byte
;     48-51  = limit high
;     52-55  = flags
;     56-63  = base high

    dq 0 ; null descriptor
gdt_kernelCode:
    dw 0xFFFF
    dw 0x0000
    db 0x0000
    db 10011010b
    db 11001111b
    db 0x0000

gdt_kernelData:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b        ; Access byte (data segment)
    db 11001111b
    db 0x00

gdt_userCode:
    dw 0xFFFF
    dw 0x0000
    db 0x0000
    db 11111010b
    db 11001111b
    db 0x0000

gdt_userData:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 11110010b        ; Access byte (data segment)
    db 11001111b
    db 0x00
gdt_end:
    KernelCodeSelector equ ((gdt_kernelCode - gdt_start) / 8) * 8
    KernelDataSelector equ ((gdt_kernelData - gdt_start) / 8) * 8
    UserCodeSelector equ ((gdt_userCode - gdt_start) / 8) * 8

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


data:
    greetingMsg db "Welcome to Lexvi's bootloader", 0 ; 0-terminated string
    diskErrorMsg db "Failed to read sector 2", 0 ; 0-terminated string

    boot_drive db 0

    ; using times command:
    ;   - times N <instruction>
    ;   - times repeats the instruction N times
    ;   - $ gives current address
    ;   - $$ address of start of the section
    times 510 - ($ - $$) db 0 ; writes 0 from here to the 510th byte
    ; 2 bytes of signature (for BIOS to recognize this as bootloader)
    db 0x55
    db 0xAA
