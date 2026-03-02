; This runs right after BIOS/Start-Up in 16-bit real mode

; 520 bytes of custom code

; We have 16-bit registers AX, BX, CX and DX
; The prefix H and L tells us to write to the upper or lower 8-bits respectively

; INT instruction -> INTERRUPT

; Setting cursor mode;
;   - AH -> bios function
;   - CH -> cursor blink(bits 5 & 6) and top most scan lines (bits 0 - 4)
;   - CL -> lower most scan lines (bits 0 - 4)
; Note: CH/CL values are scan line indexes, not pixels directly. Therefore scan line bits determine the height of the cursor
; Just to test, i want a cursor like this:
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

; Displaying characters 'LEXVI'
mov ah, 0Eh ; teletype output (prints character and advances cursor)
mov bh, 0 ; page number
; mov bl, 0b11111111 ; foreground color

mov al, 'L'
int 10h
mov al, 'E'
int 10h
mov al, 'X'
int 10h
mov al, 'V'
int 10h
mov al, 'I'
int 10h

jp $ ; jump to current address -> infinite loop

; using times command:
;   - times N <instruction>
;   - times repeats the instruction N times
;   - $ gives current address
;   - $$ address of start of the section
times 510 - ($ - $$) db 0 ; writes 0 from here to the 510th byte
; 2 bytes of signature (for BIOS to recognize this as bootloader)
db 0x55
db 0xAA
