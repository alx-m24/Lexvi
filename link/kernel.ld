ENTRY(kernel_main)

HHDM_BASE        = 0xFFFF800000000000;
KERNEL_PHYS_BASE = 0x100000;
KERNEL_VIRT_BASE = 0xFFFFFFFF80100000;  /* = 0xFFFFFFFF80000000 + 0x100000 */

PHDRS {
    text   PT_LOAD FLAGS(5);   /* r-x */
    rodata PT_LOAD FLAGS(4);   /* r-- */
    data   PT_LOAD FLAGS(6);   /* rw- */
}

SECTIONS {
    . = KERNEL_VIRT_BASE;

    .text : AT(KERNEL_PHYS_BASE) {
        KEEP(*(.text.kernel_main))
        *(.text)
        *(.text.*)
    } :text

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } :rodata

    .data : {
        *(.data)
        *(.data.*)
    } :data

    .init_array : {              /* ← BEFORE .bss */
        _init_array_start = .;
        KEEP(*(.init_array*))
        _init_array_end = .;
    } :data

    .bss : {                     /* ← AFTER .init_array */
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    } :data

    _kernel_end = .;

    KERNEL_STACK_SIZE = 0x10000;
    .stack (NOLOAD) : ALIGN(4096) {
        stack_bottom = .;
        . += KERNEL_STACK_SIZE;
        stack_top = .;
    } :data

}
