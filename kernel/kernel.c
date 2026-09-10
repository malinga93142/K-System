// kernel.c
#include "../boot/multiboot.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/lapic.h"
#include "../cpu/pic.h"
#include "../cpu/smp.h"
#include "../cpu/smp_bring.h"
#include "../drivers/uart.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../mm/mm.h"
#include "../mm/paging.h"
#include "../mm/pmm.h"
#include "kstring.h"

extern uint8_t kernel_start[];
extern uint8_t kernel_end[];
extern void setup_usermode();
// extern volatile int ap_ready;
uint32_t k_phys_start = (uint32_t)kernel_start - KERNEL_BASE;
uint32_t k_phys_end = (uint32_t)(kernel_end - KERNEL_BASE);
void verify_magic(unsigned int magic) {
  if (magic != 0x36D76289) {
    uart_hex(magic);
    uart_puts("Wrong magic number\n\r");
    asm volatile("hlt");
  }
}

// A simple polling loop, e.g. in kmain's idle loop or a dedicated shell task
// later
void input_echo_loop(void) {
  for (;;) {
    char c = keyboard_getchar();
    if (c) {
      vga_putc(c);
      uart_putc(c); 
    }
    __asm__ volatile(
        "hlt");
  }
}
void kmain(unsigned int magic, unsigned int mb_info_ptr) {
  verify_magic(magic);
  pmm_init(mb_info_ptr);
  vmm_init(k_phys_start, k_phys_end);
  init_gdt(0);
  init_idt();
  lapic_init();
  bring_all();
  pic_remap(0x20, 0x28);
  pic_mask_all();
  pit_init(100);
  pic_unmask_irq(0);
  pic_unmask_irq(1);
  asm volatile("int $0x30");
  // setup_usermode();
  vga_clear();
  asm volatile("sti");
  input_echo_loop();
  for (;;) __asm__ volatile("hlt");
}
