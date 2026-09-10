#ifndef SMP_H
#define SMP_H

#include <stdint.h>
#define MAX_APS   4
#define LAPIC_ICR_LOW 0x300
#define LAPIC_ICR_HIGH 0x310
#define AP_TRAMPOLINE_PHYS 0x8000

// #define AP_PATCH(dst, src, type, symbol)	
	// ((type *)((uintptr_t)(dst) + ((uintptr_t)&symbol)-(uintptr_t)(src))))
#define AP_PATCH(dst, src, type, symbol) \
  ((type *)((uintptr_t)(dst) + ((uintptr_t)&(symbol) - (uintptr_t)(src))))

#define READ_CR3() ({		\
	uint32_t val;         \
  asm volatile(         \
  	"mov %%cr3, %0"     \
 		: "=r"(val)         \
	);                    \
	val;                  \
})



extern volatile int aps_ready[MAX_APS];


/* Physical address where the AP real-mode trampoline gets copied and
   executed from. Must be page-aligned (SIPI vector encodes it as a
   page number) and below 1MB (APs start in real mode). */

/* Set by ap_main() once the AP has completed its own init and is ready
   to be considered "up" -- the BSP polls this after sending SIPI.
   Declared here so both smp.c (BSP side, polls it) and ap_main.c
   (AP side, sets it) can see the same symbol. */
// extern volatile int ap_ready;

/* Bring up one application processor identified by its LAPIC ID.
   Copies the trampoline to AP_TRAMPOLINE_PHYS, patches in the shared
   CR3, a fresh per-AP kernel stack, and the ap_main entry address,
   then sends the INIT-SIPI-SIPI sequence via the LAPIC. Blocks
   (busy-polls ap_ready) until the AP confirms it's running, or
   returns after some fixed number of attempts if it never does --
   see smp.c for the current (placeholder, uncalibrated) timeout
   behavior. */
void start_ap(uint32_t apic_id, uint32_t ap_slot);
extern uint8_t ap_trampoline_start[];
extern uint8_t ap_trampoline_end[];
extern uint8_t ap_idt_ptr[];
extern uint32_t ap_cr3_val;
extern uint32_t ap_stack_val;
extern uint32_t ap_main_addr;
extern uint32_t ap_slot_val;
extern void ap_main(int);

#endif /* SMP_H */
