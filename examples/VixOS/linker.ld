OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 1M;
    .text : ALIGN(8)
    {
        KEEP(*(.text.boot))
        *(.text)
        *(.text*)
    }
    .rodata : ALIGN(8) /* data only read */
    {
        *(.rodata)
        *(.rodata*)
    }
    .data : ALIGN(8)
    {
        *(.data)
        *(.data*)
    }
    .bss : ALIGN(8) /*bss*/
    {
        *(COMMON)
        *(.bss)
        *(.bss*)
    }
}
