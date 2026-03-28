OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS
{
    text    PT_LOAD    FLAGS(0x05); /* Read | Execute */
    rodata  PT_LOAD    FLAGS(0x04); /* Read */
    data    PT_LOAD    FLAGS(0x06); /* Read | Write */
}

SECTIONS
{
    /* Higher-Half Kernel Base Address */
    . = 0xFFFFFFFF80000000;

    .text : ALIGN(4K) {
        *(.text .text.*)
    } :text

    .rodata : ALIGN(4K) {
        *(.rodata .rodata.*)
    } :rodata

    .data : ALIGN(4K) {
        *(.data .data.*)
    } :data

    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss .bss.*)
    } :data
}
