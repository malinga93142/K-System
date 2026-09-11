#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
typedef struct {	volatile uint32_t locked;	} spinlock_t;
void spinlock_init(spinlock_t *lk);
void spinlock_acquire(spinlock_t *lk);
void spinlock_release(spinlock_t *lk);

uint32_t disable_local_interrupts(void);
void restore_local_interrupts(uint32_t);

uint32_t spinlock_lock_irqsave(spinlock_t *);
void spinlock_unlock_irqrestore(spinlock_t *, uint32_t);
#endif	// SPINLOCK_H
