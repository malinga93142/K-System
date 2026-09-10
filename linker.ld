ENTRY(_start)

KERNEL_VIRT_BASE = 0xC0000000;
KERNEL_PHYS_BASE = 0x00100000;

SECTIONS
{
    . = KERNEL_VIRT_BASE + KERNEL_PHYS_BASE;

    kernel_start = .;   /* virtual start */

    .text ALIGN(4096) : AT(ADDR(.text) - KERNEL_VIRT_BASE)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - KERNEL_VIRT_BASE)
    {
        *(.rodata)
    }

    .data ALIGN(4096) : AT(ADDR(.data) - KERNEL_VIRT_BASE)
    {
        *(.data)
    }

    .bss ALIGN(4096) : AT(ADDR(.bss) - KERNEL_VIRT_BASE)
    {
        *(.bss)
        *(COMMON)
    }
    kernel_end = .;     /* virtual end */
}
