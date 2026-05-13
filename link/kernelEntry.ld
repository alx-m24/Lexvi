ENTRY(kernel_entry)

SECTIONS {
    . = 0x8000;

    .text : {
        *boot-stage2.nasm.o(.text)
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .init_array : {
        _init_array_start = .;
        *(.init_array*)
        _init_array_end = .;
    }

    .bss : {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    }

    _kernel_end = .;
}

