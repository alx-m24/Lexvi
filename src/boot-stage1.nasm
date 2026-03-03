; This runs right after BIOS/Start-Up in 16-bit real mode

; We have 16-bit registers AX, BX, CX and DX
; The prefix H and L tells us to write to the upper or lower 8-bits respectively

; INT instruction -> INTERRUPT

section .text
    global _start

_start:
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

    mov ax, 0x07C0 ; segment of bootloader
    mov ds, ax ; setting ds register

    mov si, greetingMsg
    call printString

    ; loading stage-2 at absolute address 0x8000
    ; setting up buffer
    mov ax, 0x8000
    mov es, ax
    xor bx, bx ; writing 0 to bx
    ; now ES:BS = 0x8000:0000

    mov ah, 02h
    mov al, 1 ; reading 1 sector only
    mov ch, 0 ; cylinder 0
    mov cl, 2 ; sector number
    mov dh, 0 ; head 0
    ; mov dl,  ; hard disk -> already set by BIOS
    int 13h

    jc disk_error ; error -> carry flag set

    ; Successfully read sector 2/stage-2
    jmp 0x8000:0000

disk_error:
    mov si, diskErrorMsg
    call printString
    jmp $ ; jump to current address -> infinite loop

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

section .data
    greetingMsg db "Welcome to Lexvi's bootloader", 0 ; 0-terminated string
    diskErrorMsg db "Failed to read sector 2", 0 ; 0-terminated string

    ; using times command:
    ;   - times N <instruction>
    ;   - times repeats the instruction N times
    ;   - $ gives current address
    ;   - $$ address of start of the section
    times 510 - ($ - $$) db 0 ; writes 0 from here to the 510th byte
    ; 2 bytes of signature (for BIOS to recognize this as bootloader)
    db 0x55
    db 0xAA
