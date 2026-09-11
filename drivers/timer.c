#include "../cpu/pic.h"
#include "vga.h"
#include "../cpu/exception_frame.h"
volatile uint64_t timer_ticks = 0;

void timer_handler(struct exception_frame* ef) {
	(void)ef;
  timer_ticks++;
}
void delay(uint64_t wait) {
	uint64_t start = timer_ticks;
	while ((timer_ticks - start) < wait) {}
}
