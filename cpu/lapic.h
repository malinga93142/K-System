#ifndef LAPIC_H
#define LAPIC_H
#include <stdint.h>

#define LAPIC_VIRT_ADDR 0xC0500000

#define LAPIC_REG_ID             0x020
#define LAPIC_REG_VERSION        0x030

#define LAPIC_REG_TPR            0x080
#define LAPIC_REG_EOI            0x0B0
#define LAPIC_REG_SPURIOUS       0x0F0

#define LAPIC_REG_ICR_LOW        0x300
#define LAPIC_REG_ICR_HIGH       0x310

#define LAPIC_REG_TIMER_LVT      0x320
#define LAPIC_REG_TIMER_INITCNT  0x380
#define LAPIC_REG_TIMER_CURCNT   0x390
#define LAPIC_REG_TIMER_DIV      0x3E0


#define AP1_IPI_WAKE_VECTOR			0xF1

void lapic_init(void);
uint32_t lapic_read(uint32_t);
void lapic_write(uint32_t, uint32_t);
uint32_t lapic_get_id(void);
void lapic_send_eoi(void);
void lapic_send_ipi(uint32_t, uint32_t);
#endif
