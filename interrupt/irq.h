#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>
#include "../cpu/exception_frame.h"
typedef void (*irq_handler_t)(struct exception_frame *);

struct irq_entry {
    irq_handler_t handler;
    uint8_t irq;
    uint8_t enabled;
};

#define IRQ_COUNT   0x10

void irq_init(void);
void irq_register(struct irq_entry *, uint8_t, irq_handler_t handler);
void irq_dispatch(struct exception_frame *ef);
void irq_handler(struct exception_frame *);

#endif