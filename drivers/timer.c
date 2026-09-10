#include "../cpu/pic.h"
#include "vga.h"
volatile uint64_t timer_ticks = 0;

void timer_handler(struct exception_frame* ef) {
  timer_ticks++;

  if (timer_ticks > 10) {
    vga_puts("timer interrupt\n");
    vga_putc('\r');
    pic_mask_irq(0);
  }
}