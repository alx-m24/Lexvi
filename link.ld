ENTRY(kernel_entry)

SECTIONS {
    . = 0x8000;
    .text : {
        build/boot-stage2.o(.text)
        *(.text)
    }
    .data : {
        *(.data)
        *(.rodata)
    }
    .bss : {
        *(.bss)
    }
}
