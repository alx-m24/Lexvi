ENTRY(kernel_main)

SECTIONS {
    . = 0x100000;
    .text : {
        KEEP(*(.text.kernel_main))
        *(.text)
    }
    .data : {
        *(.data)
        *(.rodata)
    }
    .bss : {
        *(.bss)
        *(COMMON)
    }
}
