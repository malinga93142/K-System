#include "spinlock.h"

void spinlock_init(spinlock_t *lk)	{lk->locked=0;}
void spinlock_acquire(spinlock_t *lk)
{
	while (__sync_lock_test_and_set(&lk->locked, 1)) {
		while (lk->locked)	asm volatile("pause");
	}
}
void spinlock_release(spinlock_t *lk) {
	__sync_lock_release(&lk->locked);
}

uint32_t disable_local_interrupts(void) {
	uint32_t eflags;
	asm volatile (
		"pushfl\n\t"
		"popl %0\n\t"
		"cli\n\t"
		:"=r"(eflags)
		::"memory"
	);
	return eflags;
}
void restore_local_interrupts(uint32_t eflags) {
	if (eflags & (1 << 9)) {
		asm volatile ("sti":::"memory");
	}
}
uint32_t spinlock_lock_irqsave(spinlock_t *lock) {
	uint32_t flags;
	asm volatile(
		"pushfl\n\t"
		"popl %0"
		:"=r"(flags)
		::"memory"
	);
	while (__builtin_expect(1, 1)) {
		uint32_t oldval = 1;
		asm volatile (
			"xchgl %0, %1"
			:"+r"(oldval), "+m"(lock->locked)
			::"memory"
		);
		if (oldval == 0)	break;
		asm volatile ("pause":::"memory");
	}
	return flags;
}
void spinlock_unlock_irqrestore(spinlock_t *lock, uint32_t flags) {
	asm volatile ("movl $0, %0":"=m"(lock->locked)::"memory");
	asm volatile(
		"pushl %0\n\t"
		"popfl"
		::"rm"(flags):"memory", "cc"
	);
}
