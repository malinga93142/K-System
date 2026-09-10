# ap_trampoline.S
.intel_syntax noprefix
.code16
.section .text
.global ap_trampoline_start
.global ap_trampoline_end

ap_trampoline_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    lgdt [ap_gdt_descriptor - ap_trampoline_start + 0x8000]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ljmp 0x08, 0x8000 + (ap_protected - ap_trampoline_start)

.code32
ap_protected:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov eax, [0x8000 + (ap_cr3_val - ap_trampoline_start)]
    mov cr3, eax
    lidt [0x8000 + (ap_idt_ptr - ap_trampoline_start)]

    mov eax, cr4
    or eax, 0x10          # PSE -- harmless even if unused by final tables
    mov cr4, eax

    /* cr0 was here */
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    mov esp, [0x8000 + (ap_stack_val - ap_trampoline_start)]

    /* jump to a real, high-half, correctly-linked kernel symbol --
       this works because paging is now on and the shared kernel page
       tables (loaded via ap_cr3_val above) already map this address
       correctly, exactly like your very first boot.S's higher_half jump */
		mov ebx, ap_main_addr - ap_trampoline_start + 0x8000
    mov ebx, [ebx]
    mov eax, [(ap_slot_val - ap_trampoline_start + 0x8000)]
    push eax
    call ebx

.align 8
ap_gdt_start:
    .quad 0
    .word 0xFFFF, 0
    .byte 0, 0x9A, 0xCF, 0
    .word 0xFFFF, 0
    .byte 0, 0x92, 0xCF, 0
ap_gdt_end:

ap_gdt_descriptor:
    .word ap_gdt_end - ap_gdt_start - 1
    .long ap_gdt_start - ap_trampoline_start + 0x8000

.global ap_cr3_val
ap_cr3_val:      .long 0    # BSP patches this before sending SIPI

.global ap_stack_val
ap_stack_val:    .long 0    # BSP patches this

.global ap_main_addr
ap_main_addr:    .long 0    # BSP patches this with ap_mains real, linked address

.globl ap_idt_ptr
ap_idt_ptr:
    .word 0
    .long 0
.globl ap_slot_val
ap_slot_val:	.long 0
ap_trampoline_end:
