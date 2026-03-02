; AxiomOS Kernel Linker Script
; x86_64 Higher-Half Kernel (0xFFFFFFFF80000000)

ENTRY(kentry)

SECTIONS {
    . = 0xFFFFFFFF80000000;

    .text ALIGN(0x1000) : AT(ADDR(.text) - 0xFFFFFFFF80000000) {
        *(.text.entry)
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - 0xFFFFFFFF80000000) {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - 0xFFFFFFFF80000000) {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xFFFFFFFF80000000) {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
