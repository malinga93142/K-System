#ifndef LAPIC_H
#define LAPIC_H
#include <stdint.h>

void lapic_init(void);
uint32_t lapic_get_id(void);
void lapic_send_eoi(void);
void lapic_write(uint32_t, uint32_t);
uint32_t lapic_read(uint32_t);
#endif
