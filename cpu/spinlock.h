#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
typedef struct {	volatile uint32_t locked;	} spinlock_t;
void spinlock_init(spinlock_t *lk);
void spinlock_acquire(spinlock_t *lk);
void spinlock_release(spinlock_t *lk);
#endif	// SPINLOCK_H
