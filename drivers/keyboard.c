#include "../cpu/pic.h"
#include "uart.h"
#include "../io/io.h"
void keyboard_handler(struct exception_frame *ef)
{
  uint8_t scancode = inb(0x60);

  uart_puts("KBD scancode=");
  uart_hex(scancode);
}