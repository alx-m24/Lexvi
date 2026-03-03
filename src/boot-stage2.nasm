section .text
    global _start

_start:
    ; Setting cursor position
    mov ah, 02h
    mov bh, 00h ; page number & graphics mode
    mov dh, 02h ; row (00h is top)
    mov dl, 00h ; column (00h is left)
    int 10h
    
    ; Printing welcome msg
    mov ax, cs
    mov ds, ax

    mov si, greetingMsg
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
    greetingMsg db "Welcome to Lexvi's bootloader stage 2", 0 ; 0-terminated string
