.set KERNEL_VIRT_BASE, 0xC0000000
.section .multiboot
.align 8
.set MB2_MAGIC,    0xE85250D6
.set MB2_ARCH,     0
.set MB2_LENGTH,   mb2_end - mb2_start
.set MB2_CHECKSUM, -(MB2_MAGIC + MB2_ARCH + MB2_LENGTH)

mb2_start:
    .long MB2_MAGIC
    .long MB2_ARCH
    .long MB2_LENGTH
    .long MB2_CHECKSUM
    # end tag
    .short 0
    .short 0
    .long 8
mb2_end:

.section .text
.global _start
.extern kmain
_start:
    cli
    mov %eax, %edi          # stash magic
    mov %ebx, %esi          # stash mb_info ptr (still physical, fine for now)
    lgdt (gdt_descriptor - KERNEL_VIRT_BASE)
    mov $0x10, %ax           # data selector = index 2 * 8
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $flush_cs - KERNEL_VIRT_BASE
flush_cs:
    mov $(boot_page_dir - KERNEL_VIRT_BASE), %ecx
    movl $(0x00000000 | 0x83), (%ecx)        # PDE[0]:   identity 0-4MB
    movl $(0x00000000 | 0x83), 768*4(%ecx)   # PDE[768]: 0xC0000000-0xC03FFFFF -> 0-4MB
    mov %ecx, %cr3
    mov %cr4, %eax
    or  $0x00000010, %eax
    mov %eax, %cr4
    mov %cr0, %eax
    or  $0x80000000, %eax
    mov %eax, %cr0
    lea higher_half, %eax
    jmp *%eax
higher_half:
    mov $stack_top, %esp
    push %esi
    push %edi
    call kmain
hang:
    hlt
    jmp hang
.section .data
.align 8
gdt_start:
    .quad 0x0000000000000000
    .quad 0x00CF9A000000FFFF
    .quad 0x00CF92000000FFFF
gdt_end:
gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start - KERNEL_VIRT_BASE

.align 4096
boot_page_dir:
    .fill 1024, 4, 0
.section .bss
.align 16
stack_bottom:
    .skip 65536
stack_top:
