// ap_main.c
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/lapic.h"
#include "../cpu/smp.h"
#include "../cpu/cpu.h"
#include "../cpu/exception_frame.h"
#include "../drivers/uart.h"
#include "../drivers/vga.h"
extern void setup_usermode();

void ipi_handler(struct exception_frame *ef) {
	(void)ef;
	uart_puts("\nwoke up\n");
	lapic_write(LAPIC_REG_EOI, 0);
}
void ap_main(int ap_slot) {
	uint8_t lapic_id = (uint8_t)(lapic_read(LAPIC_REG_ID));
	cpus[ap_slot].cpu_id = ap_slot;
	cpus[ap_slot].lapic_id=lapic_id;
  init_gdt(ap_slot);
  init_idt();
  aps_ready[ap_slot] = 1;
  setup_usermode();
  asm volatile("sti");
  for (;;) __asm__ volatile("hlt");
}
