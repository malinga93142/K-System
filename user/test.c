#include <stdint.h>
#include "../drivers/uart.h"
#include "../boot/multiboot.h"
extern void *get_module_data(uint32_t, uint32_t*);
void test_code_initrd() {
  // asm volatile("int $64");
  uint32_t size;
  void *data = get_module_data(0, &size);
  uart_hex((uintptr_t)data); uart_putc(0xa);
  if (data) {
    uart_puts("module 0 size=");
    uart_hex(size);
    uart_puts("\n");
    uart_puts("first bytes: ");
    uint8_t *bytes = (uint8_t *)data;
    for (int i = 0; i < (int)size; i++) {
      uart_putc(bytes[i]);
      uart_putc(' ');
    }
    uart_puts("\n");
  }
}
