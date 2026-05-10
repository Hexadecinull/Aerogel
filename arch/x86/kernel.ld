OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)
ENTRY(_start)

SECTIONS {
    . = 0x10000;

    .text : {
        KEEP(*(.text.entry))
        *(.text .text.*)
    }

    .rodata ALIGN(4096) : {
        *(.rodata .rodata.*)
    }

    .data ALIGN(4096) : {
        *(.data .data.*)
    }

    _bss_start = .;
    .bss ALIGN(4096) : {
        *(COMMON)
        *(.bss .bss.*)
    }
    _bss_end = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
        *(.note*)
    }
}
