#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1 0x20
#define PIC2 0xA0

#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)

#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20
#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40

void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_mask_all(void);
void pic_unmask_irq(uint8_t irq);
void pic_mask_irq(uint8_t irq);
void pic_send_eoi(uint8_t irq);

void pit_init(uint32_t frequency);

#endif
