#ifndef IDT_H
#define IDT_H
#include <stdint.h>
struct idt_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t type_attr;
  uint16_t offset_high;
} __attribute__((packed));

struct idtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));
extern struct idtr idtptr;
extern void isr8();
extern void isr14();
extern void isr48();
extern void isr64();
extern void isr65();

/* IRQs */
extern void isr32();
extern void isr33();
void init_idt();
#endif
