; org 0x8000

[bits 32]

; forward declaration
extern kernel_entry

stage2_start:
    cld
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov cl, 0
    mov ch, 1
    mov bl, 0x0F
    mov esi, greetingMsg
    call print_string_vga

    call checkCPUID
    cmp eax, 1
    je cpuidAvailable
    mov cl, 0
    mov ch, 2
    mov bl, 0x0F
    mov esi, cpuidNotSupportedMsg
    call print_string_vga
    jmp hang

cpuidAvailable:
    mov cl, 0
    mov ch, 2
    mov bl, 0x0F
    mov esi, cpuidSupportedMsg
    call print_string_vga

queryLongMode:
    mov eax, CPUID_EXTENSIONS
    cpuid
    cmp eax, CPUID_EXT_FEATURES
    jb .NoLongMode              ; if the CPU can't report long mode support, then it likely doesn't have it
    mov eax, CPUID_EXT_FEATURES
    cpuid
    test edx, CPUID_EDX_EXT_FEAT_LM
    jz .NoLongMode
    jmp .LongMode

.NoLongMode:
    mov cl, 0
    mov ch, 3
    mov bl, 0x0F
    mov esi, longModeNotSupportedMsg
    call print_string_vga
    jmp hang
.LongMode:
    mov cl, 0
    mov ch, 3
    mov bl, 0x0F
    mov esi, longModeSupportedMsg
    call print_string_vga

.setupPaging:
    mov edi, PML4T_ADDR
    mov cr3, edi       ; cr3 lets the CPU know where the page tables are

    xor eax, eax
    mov ecx, (5 * SIZEOF_PAGE_TABLE) / 4
    rep stosd          ; writes 4 * SIZEOF_PAGE_TABLE bytes, which is enough space
                       ; for the 4 page tables
    mov edi, cr3       ; reset di back to the beginning of the page table
    
    mov DWORD [edi], PDPT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE

    mov edi, PDPT_ADDR
    mov DWORD [edi], PDT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE

    mov edi, PDT_ADDR
    mov DWORD [edi], PT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE

    mov edi, PDT_ADDR
    mov DWORD [edi],        PT_ADDR  & PT_ADDR_MASK | PT_PRESENT | PT_READABLE
    mov DWORD [edi + 8],    PT2_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE

.fillPageTable:
    mov edi, PT_ADDR
    mov ebx, PT_PRESENT | PT_READABLE
    mov ecx, ENTRIES_PER_PT      ; 1 full page table addresses 2MiB

.SetEntry:
    mov DWORD [edi], ebx
    add ebx, PAGE_SIZE
    add edi, SIZEOF_PT_ENTRY
    loop .SetEntry               ; Set the next entry.

    mov edi, PT2_ADDR
    mov ebx, 0x200000 | PT_PRESENT | PT_READABLE
    mov ecx, ENTRIES_PER_PT
.SetEntry2:
    mov DWORD [edi], ebx
    add ebx, PAGE_SIZE
    add edi, SIZEOF_PT_ENTRY
    loop .SetEntry2

.enable_PAE:
    mov eax, cr4
    or eax, CR4_PAE_ENABLE
    mov cr4, eax

    mov ecx, EFER_MSR
    rdmsr
    or eax, EFER_LM_ENABLE
    wrmsr

    mov eax, cr0
    or eax, CR0_PG_ENABLE | CR0_PM_ENABLE   ; ensuring that PM is set will allow for jumping
                                            ; from real mode to compatibility mode directly
    mov cr0, eax

.Enabling_64Bits_Submode:
    mov cl, 0 
    mov ch, 4
    mov bl, 0x0F
    mov esi, EnablingLongModeMSG
    call print_string_vga

    lgdt [GDT.Pointer]
    jmp GDT.Code:Realm64

hang:
    jmp hang

; Checks if CPUID is supported by attempting to flip the ID bit (bit 21) in
; the EFLAGS register. If we can flip it, CPUID is available.
; returns eax = 1 if there is cpuid support; 0 otherwise
checkCPUID:
    pushfd
    pop eax

    ; The original value should be saved for comparison and restoration later
    mov ecx, eax
    xor eax, EFLAGS_ID

    ; storing the eflags and then retrieving it again will show whether or not
    ; the bit could successfully be flipped
    push eax                    ; save to eflags
    popfd
    pushfd                      ; restore from eflags
    pop eax

    ; Restore EFLAGS to its original value
    push ecx
    popfd

    ; if the bit in eax was successfully flipped (eax != ecx), CPUID is supported.
    xor eax, ecx
    jnz .supported
    .notSupported:
        mov eax, 0
        ret
    .supported:
        mov eax, 1
        ret

print_char_vga:
    push eax
    mov edi, VGA_MEMORY     ; load base into register
    mov edx, ecx
    shl edx, 1
    add edi, edx            ; edi = exact byte address in VGA buffer
    mov ah, bl
    mov [edi], ax           ; write char + color
    pop eax
    ret

; cl -> col, ch -> row
print_string_vga:
    ; index = col + row * 80
    movzx eax, ch
    imul eax, 80
    movzx edx, cl
    add eax, edx
    mov ecx, eax

.next_char:
    lodsb
    cmp al, 0
    je .done
    call print_char_vga
    inc ecx
    jmp .next_char
.done:
    ret

_data:
    VGA_MEMORY equ 0xB8000
    greetingMsg db "Welcome to Lexvi's stage 2. Successfully switched to Protected mode(32-bit)", 0
    cpuidSupportedMsg db "   - CPUID is supported", 0
    cpuidNotSupportedMsg db "   - CPUID is NOT supported", 0
    longModeSupportedMsg db "   - CPU supports Long Mode", 0
    longModeNotSupportedMsg db "   - CPU does NOT support Long Mode", 0
    EnablingLongModeMSG db "Enabling long mode...", 0
    realm64Msg db "Successfully entered 64-bit long mode", 0

    EFLAGS_ID equ 1 << 21 ; if this bit can be flipped, the CPUID instruction is available
    CPUID_EDX_EXT_FEAT_LM equ 1 << 29   ; if this is set, the CPU supports long mode
    CPUID_EXTENSIONS equ 0x80000000 ; returns the maximum extended requests for cpuid
    CPUID_EXT_FEATURES equ 0x80000001

    SIZEOF_PAGE_TABLE equ 4096
    PML4T_ADDR equ 0x1000
    PDPT_ADDR equ 0x2000
    PDT_ADDR equ 0x3000
    PT_ADDR equ 0x4000

    PT2_ADDR equ 0x5000

    ; the page table only uses certain parts of the actual address
    PT_ADDR_MASK equ 0xffffffffff000
    PT_PRESENT equ 1                 ; marks the entry as in use
    PT_READABLE equ 2                ; marks the entry as r/w

    ENTRIES_PER_PT equ 512
    SIZEOF_PT_ENTRY equ 8
    PAGE_SIZE equ 0x1000

    CR4_PAE_ENABLE equ 1 << 5

    EFER_MSR equ 0xC0000080
    EFER_LM_ENABLE equ 1 << 8
    CR0_PM_ENABLE equ 1 << 0
    CR0_PG_ENABLE equ 1 << 31

    ; Access bits
    PRESENT        equ 1 << 7
    NOT_SYS        equ 1 << 4
    EXEC           equ 1 << 3
    DC             equ 1 << 2
    RW             equ 1 << 1
    ACCESSED       equ 1 << 0
    
    ; Flags bits
    GRAN_4K       equ 1 << 7
    SZ_32         equ 1 << 6
    LONG_MODE     equ 1 << 5
    
    GDT:
        .Null: equ $ - GDT
            dq 0
        .Code: equ $ - GDT
            .Code.limit_lo: dw 0xffff
            .Code.base_lo: dw 0
            .Code.base_mid: db 0
            .Code.access: db PRESENT | NOT_SYS | EXEC | RW
            .Code.flags: db GRAN_4K | LONG_MODE | 0xF   ; Flags & Limit (high, bits 16-19)
            .Code.base_hi: db 0
        .Data: equ $ - GDT
            .Data.limit_lo: dw 0xffff
            .Data.base_lo: dw 0
            .Data.base_mid: db 0
            .Data.access: db PRESENT | NOT_SYS | RW
            .Data.Flags: db GRAN_4K | SZ_32 | 0xF       ; Flags & Limit (high, bits 16-19)
            .Data.base_hi: db 0
        .Pointer:
            dw $ - GDT - 1
            dd GDT

[bits 64]
Realm64:
    mov ax, GDT.Data
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor ax, ax
    mov fs, ax
    mov gs, ax
    
    mov r8b, 0
    mov r9b, 5
    mov bl, 0x0F
    mov rsi, realm64Msg
    call print_string_vga64

    call kernel_entry

.hang:
    jmp .hang

; r8b -> col, r9b -> row, bl -> color, rsi -> string
print_string_vga64:
    movzx rax, r9b
    imul rax, 80
    movzx rdx, r8b
    add rax, rdx
    mov rcx, rax
.next_char:
    lodsb
    cmp al, 0
    je .done
    call print_char_vga64
    inc rcx
    jmp .next_char
.done:
    ret

print_char_vga64:
    push rax
    mov rdi, VGA_MEMORY
    mov rdx, rcx
    shl rdx, 1
    add rdi, rdx
    mov ah, bl
    mov [rdi], ax
    pop rax
    ret
