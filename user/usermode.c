/* usermode.c */
#include <stdint.h>
#include "../mm/paging.h"
#include "../mm/pmm.h"
#include "../kernel/kstring.h"
#include "../drivers/uart.h"
extern void enter_usermode(uint32_t entry, uint32_t user_stack);
extern char test_code_start[];
extern char test_code_end[];
void setup_usermode()
{
  uint32_t code_vaddr = 0x08048000;
  uint32_t stack_top = 0x08051000;
  uint32_t stack_size = PAGE_SIZE;

  uint32_t code_size = (uint32_t)&test_code_end - (uint32_t)&test_code_start;

  if (uvm_load_page(code_vaddr, (void *)&test_code_start, code_size) != 0)
  {
    uart_puts("setup_and_enter_usermode:uvm_load_page: failed\n");
    return;
  }
  if (uvm_create_stack(stack_top, stack_size) != 0)
  {
    uart_puts("setup_and_enter_usermode:uvm_create_stack: failed\n");
    return;
  }
  uart_puts("entering into usermode");
  uint32_t cr3;
  asm volatile("mov %%cr3, %0" : "=r"(cr3));
  asm volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
  enter_usermode(code_vaddr, stack_top);
}
