#include "smp.h"
void bring_all() {
  /* APIC ID, SLOT */
  // asm volatile("sti");
  start_ap(1, 1);
  // start_ap(2, 2);
  // start_ap(3, 3);
}
