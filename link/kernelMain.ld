ENTRY(kernel_main)

PHDRS {
    text   PT_LOAD FLAGS(5);   /* r-x */
    rodata PT_LOAD FLAGS(4);   /* r-- */
    data   PT_LOAD FLAGS(6);   /* rw- */
}

SECTIONS {
    . = 0x100000;

    .text : {
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

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } :data

    .init_array : {
        _init_array_start = .;
        *(.init_array*)
        _init_array_end = .;
    } :data

    .stack (NOLOAD) : ALIGN(4096) {
        stack_bottom = .;
        . += 0x10000;
        stack_top = .;
    } :data

    _kernel_end = .;
}
