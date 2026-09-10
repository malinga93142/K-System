#include "uart.h"
#include "../io/io.h"
#include "../cpu/spinlock.h"
static spinlock_t uart_lock;
static void uart_wait(void) {
  while (!(inb(COM1 + 5) & 0x20))
    ;
}

void uart_init(void) {
  outb(COM1 + 1, 0x00); // Disable interrupts

  outb(COM1 + 3, 0x80); // Enable DLAB
  outb(COM1 + 0, 0x03); // Baud divisor low (38400)
  outb(COM1 + 1, 0x00); // Baud divisor high

  outb(COM1 + 3, 0x03); // 8 data bits, no parity, 1 stop bit
  outb(COM1 + 2, 0xC7); // Enable FIFO, clear buffers
  outb(COM1 + 4, 0x0B); // IRQs disabled, RTS/DSR enabled
}

void uart_putc(char c) {
  uart_wait();
	spinlock_acquire(&uart_lock);
  outb(COM1, (uint8_t)c);
  spinlock_release(&uart_lock);
}

void uart_puts(const char *s) {
  while (*s)
    uart_putc(*s++);
}

void uart_hex(uint32_t value) {
  static const char digits[] = "0123456789ABCDEF";

  uart_puts("0x");

  for (int shift = 28; shift >= 0; shift -= 4) {
    uart_putc(digits[(value >> shift) & 0xF]);
  }
}

void uart_dec(uint32_t value) {
  char buffer[11];
  int i = 0;

  if (value == 0) {
    uart_putc('0');
    return;
  }

  while (value) {
    buffer[i++] = '0' + (value % 10);
    value /= 10;
  }

  while (i > 0) {
    uart_putc(buffer[--i]);
  }
}
static void uart_hex32_nibbles(uint32_t value) {
  static const char digits[] = "0123456789ABCDEF";
  for (int shift = 28; shift >= 0; shift -= 4) {
    uart_putc(digits[(value >> shift) & 0xF]);
  }
}
void uart_hex64(uint64_t val) {
  uart_puts("0x");
  uart_hex32_nibbles((uint32_t)(val >> 32));        // high 32 bits
  uart_hex32_nibbles((uint32_t)(val & 0xFFFFFFFF)); // low 32 bits
}
