// kernel.c
#include "../boot/multiboot.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/lapic.h"
#include "../cpu/pic.h"
#include "../cpu/smp.h"
#include "../cpu/cpu.h"
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
extern void delay(uint64_t);
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
  // asm volatile("cli");
  verify_magic(magic);
  pmm_init(mb_info_ptr);
  vmm_init(k_phys_start, k_phys_end);
  lapic_init();
  initcpus();
  cpus[0].cpu_id=0;
  cpus[0].lapic_id = lapic_get_id();
  init_gdt(0);
  init_idt();
  pic_remap(0x20, 0x28);
  pic_mask_all();
  pit_init(100);
  pic_unmask_irq(0);
  pic_unmask_irq(1);
#if defined(AVL_SMP) && AVL_SMP==2
  bring_all();
#endif
  asm volatile("int $0x30");
  asm volatile("sti");
  delay(100);
  uart_puts("Hi after 1 second");
  asm volatile("sti");
  // vga_clear();
  // input_echo_loop();
  lapic_send_ipi(cpus[1].lapic_id, AP1_IPI_WAKE_VECTOR);
  for (;;) __asm__ volatile("hlt");
}
