#include <stdint.h>

#include "../cpu/pic.h"
#include "../drivers/uart.h"
#include "../drivers/vga.h"
#include "../interrupt/irq.h"
#include "../interrupt/syscalls.h"
#include "../io/io.h"
#include "../mm/paging.h"
#include "../mm/pmm.h"
#include "exception_frame.h"

/* forward declaration */
void double_fault(struct exception_frame* ef);

static inline uint32_t read_cr2(void) {
  uint32_t val;
  asm volatile("mov %%cr2, %0" : "=r"(val));
  return val;
}

void exception_handler(struct exception_frame* ef) {
  switch (ef->int_no) {
    case 8:
      double_fault(ef);
      break;

    case 14: {
      uint32_t fault_addr = read_cr2();
      uint32_t page_base = fault_addr & ~0xfff;
      uart_puts("PAGE FAULT\n");
      uart_puts("  cr2 (fault addr) = ");
      uart_hex(fault_addr);
      uart_puts("\n");
      uart_puts("  eip              = ");
      uart_hex(ef->eip);
      uart_puts("\n");
      uart_puts("  error_code       = ");
      uart_hex(ef->error_code);
      uart_puts("\n");
      uart_puts("  eax              = ");
      uart_hex(ef->eax);
      uart_puts("\n");
      if (ef->error_code & 0x1) {
        uart_puts("PAGE FAULT: protection fault, not auto-mapping. halted");
        while (1) asm volatile("hlt");
      }

      uint32_t frame = pmm_alloc_frame();
      if (frame == 0) {
        uart_puts("PAGE FAULT: OOM, cannot map. halting");
        while (1) asm volatile("hlt");
      }
      vmm_map_page_post_switch(page_base, frame, PAGE_RW);
      uart_puts("mapped\n");
      uart_hex(page_base);
      break;
    }
    case 32:
    case 33:
      irq_handler(ef);
      break;
    case 48:
    case 64:
    case 65:
      syscall_handler(ef);
      break;
    default:
      uart_puts("UNHANDLED INT ");
      uart_hex(ef->int_no);
      uart_puts("\n");
      break;
  }
}

void double_fault(struct exception_frame* ef) {
  uart_puts("DOUBLE FAULT eip=");
  uart_hex(ef->eip);
  uart_puts("\n");
  while (1) asm volatile("hlt");
}

// void keyboard_handler(struct exception_frame *ef)
// {
//   uint8_t scancode = inb(0x60);

//   uart_puts("KBD scancode=");
//   uart_hex(scancode);
//   uart_puts("\n");

//   pic_send_eoi(1);
// }