#include "idt.h"

#include "../drivers/uart.h"
#include "../interrupt/irq.h"
static struct idt_entry idt32[256];
struct idtr idtptr;

void setgate(struct idt_entry* e, uint32_t handler, uint16_t selector,
             uint8_t type) {
  e->offset_low = (handler) & 0xffff;
  e->offset_high = (handler >> 16) & 0xffff;
  e->selector = selector;
  e->zero = 0;
  e->type_attr = type;
}

void init_idt() {
  idtptr.limit = sizeof(idt32) - 1;
  idtptr.base = (uint32_t)(uintptr_t)&idt32;

  setgate(&idt32[8], (uint32_t)(uintptr_t)isr8, 0x8, 0x8e);
  setgate(&idt32[14], (uint32_t)(uintptr_t)isr14, 0x8, 0x8e);
  setgate(&idt32[32], (uint32_t)(uintptr_t)isr32, 0x8, 0x8e);
  setgate(&idt32[33], (uint32_t)(uintptr_t)isr33, 0x8, 0xee);
  setgate(&idt32[48], (uint32_t)(uintptr_t)isr48, 0x8, 0x8e);
  setgate(&idt32[64], (uint32_t)(uintptr_t)isr64, 0x8, 0xee);
  setgate(&idt32[65], (uint32_t)(uintptr_t)isr65, 0x8, 0xee);



  setgate(&idt32[0xF1], (uint32_t)(uintptr_t)isr241, 0x8, 0x8e);

  asm volatile("lidt %0" ::"m"(idtptr) : "memory");
  irq_init();
}
