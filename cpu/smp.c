// smp.c
#include "../drivers/uart.h"
#include "../mm/paging.h"
#include "../mm/pmm.h"
#include "../kernel/kstring.h"
#include "lapic.h"
#include "idt.h"
#include "smp.h"
#include "spinlock.h"

#define TRAMP	0xC0600000

volatile int aps_ready[MAX_APS];

static void lapic_send_ipi(uint32_t apic_id, uint32_t icr_low_value)
{
  lapic_write(LAPIC_ICR_HIGH, apic_id << 24);
  lapic_write(LAPIC_ICR_LOW, icr_low_value);

  /* wait for delivery -- poll the delivery-status bit (bit 12) until clear */
  while (lapic_read(LAPIC_ICR_LOW) & (1 << 12))
    ;
}
static spinlock_t lock = {0};

void map() {
	uint32_t tramp_size = ap_trampoline_end - ap_trampoline_start;
	pmm_reserve_region(AP_TRAMPOLINE_PHYS, PAGE_ALIGN_UP(tramp_size));
	vmm_map_page_post_switch(0xC0600000, AP_TRAMPOLINE_PHYS, PAGE_RW);
	memcpy((void *)0xC0600000, ap_trampoline_start, tramp_size);
	vmm_map_page_post_switch(AP_TRAMPOLINE_PHYS, AP_TRAMPOLINE_PHYS, PAGE_RW);

}
void start_ap(uint32_t apic_id, uint32_t ap_slot) {
	spinlock_acquire(&lock);
  /* reserve and copy the trampoline to its fixed low physical address */
	map();
  uint32_t cr3_now = READ_CR3();
  *AP_PATCH(TRAMP, ap_trampoline_start, uint32_t, ap_cr3_val		) 	= cr3_now;
  *AP_PATCH(TRAMP, ap_trampoline_start, struct idtr, ap_idt_ptr	) 	= idtptr;  
  *AP_PATCH(TRAMP, ap_trampoline_start, uint32_t, ap_stack_val	) 	= kvm_create_guarded_stack(0xC0900000 + ap_slot * 0x1000, 0x4000); /* fresh AP kernel stack */
  *AP_PATCH(TRAMP, ap_trampoline_start, uint32_t, ap_slot_val		) 	= ap_slot;
  *AP_PATCH(TRAMP, ap_trampoline_start, uint32_t, ap_main_addr	) 	= (uint32_t)(uintptr_t)ap_main;



	aps_ready[ap_slot] = 0;



  /* 3. INIT-SIPI-SIPI sequence, per Intel's documented procedure */
  lapic_send_ipi(apic_id, 0x00004500); /* INIT, level assert */

  /* brief delay needed here -- busy-wait loop, no real timer dependency yet */
  for (volatile int i = 0; i < 1000000; i++)
    ;

  lapic_send_ipi(apic_id, 0x00004600 | (AP_TRAMPOLINE_PHYS >>
                                        12)); /* SIPI, vector = page number */
  for (volatile int i = 0; i < 1000000; i++)
    ;

  lapic_send_ipi(apic_id, 0x00004600 | (AP_TRAMPOLINE_PHYS >>
                                        12)); /* SIPI again, per spec */
  uint32_t timeout = 10000000;
	while (!aps_ready[ap_slot] && timeout--);
  spinlock_release(&lock);
} // smp.c
