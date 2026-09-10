#ifndef VGA_H
#define VGA_H
#include <stdint.h>
#define VGA_BUFFER ((volatile uint16_t*)0xC00B8000)
#define VGA_COLS 80
#define VGA_ROWS 25
void vga_puts(const char*);
void vga_putc(char c);
void vga_clear();
#endif