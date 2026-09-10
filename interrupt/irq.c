#include "../cpu/exception_frame.h"
#include "../cpu/pic.h"
#include "irq.h"
extern void timer_handler(struct exception_frame *);
extern void keyboard_handler(struct exception_frame *);
struct irq_entry irq_table[IRQ_COUNT];
void irq_init() {
  for (int i = 0; i < IRQ_COUNT; i++) {
    irq_table[i].enabled = 0;
    irq_table[i].irq     = 0;
    irq_table[i].handler = 0;
  }
  irq_register(&irq_table[0], 32, timer_handler);
  irq_register(&irq_table[1], 33, keyboard_handler);
}
void irq_register(struct irq_entry *irq, uint8_t irq_no, irq_handler_t handler) {
  irq->enabled = 1;
  irq->handler = handler;
  irq->irq     = irq_no;
}
void irq_dispatch(struct exception_frame *ef) {
  uint8_t irq = ef->int_no - 32;
  if (irq < IRQ_COUNT && irq_table[irq].enabled && irq_table[irq].handler)
    (irq_table[ef->int_no-32].handler)(ef);
  pic_send_eoi(ef->int_no-32);
}
void irq_handler(struct exception_frame *ef)
{
  irq_dispatch(ef);
}