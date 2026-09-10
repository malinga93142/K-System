#ifndef UART_H
#define UART_H

#include <stdint.h>
#define COM1 0x3F8
/* UART registers */
#define UART_RBR(base) (base + 0)  // Receive Buffer Register
#define UART_THR(base) (base + 0)  // Transmit Holding Register
#define UART_IER(base) (base + 1)  // Interrupt Enable Register
#define UART_IIR(base) (base + 2)  // Interrupt Identification Register
#define UART_FCR(base) (base + 2)  // FIFO Control Register
#define UART_LCR(base) (base + 3)  // Line Control Register
#define UART_MCR(base) (base + 4)  // Modem Control Register
#define UART_LSR(base) (base + 5)  // Line Status Register
#define UART_MSR(base) (base + 6)  // Modem Status Register
#define UART_SCR(base) (base + 7)  // Scratch Register
/* LCR bits */
#define UART_LCR_DLAB 0x80
#define UART_LCR_8BIT 0x03
/* LSR bits */
#define UART_LSR_DATA_READY 0x01
#define UART_LSR_THR_EMPTY 0x20
#define UART_LSR_TX_EMPTY 0x40
/* IER bits */
#define UART_IER_RX_ENABLE 0x01
/* FCR bits */
#define UART_FCR_ENABLE 0x01
#define UART_FCR_CLEAR_RX 0x02
#define UART_FCR_CLEAR_TX 0x04

void uart_init(void);

void uart_putc(char c);
void uart_puts(const char* s);
uint8_t uart_getc(void);

void uart_hex(uint32_t value);
void uart_dec(uint32_t value);
void uart_hex64(uint64_t val);
#endif
