#ifndef UART_H
#define UART_H

#include <stdint.h>

#define COM1 0x3F8

void uart_init(void);

void uart_putc(char c);
void uart_puts(const char *s);

void uart_hex(uint32_t value);
void uart_dec(uint32_t value);
void uart_hex64(uint64_t val);
#endif
