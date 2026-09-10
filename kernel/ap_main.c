// ap_main.c
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/lapic.h"
#include "../cpu/smp.h"
#include "../drivers/uart.h"
#include "../drivers/vga.h"
void ap_main(int a) {
	init_gdt(a);
	init_idt();
	
	aps_ready[a] = 1;
  for (;;) __asm__ volatile("hlt");
}
