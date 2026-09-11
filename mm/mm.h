#ifndef MM_H
#define MM_H

#include <stdint.h>

#define PAGE_SIZE 0x1000U
#define PAGE_SHIFT 12  // 12 bits

#define USER_BASE 0x00000000U
#define USER_TOP 0xBFFFFFFFU

#define KERNEL_BASE 0xC0000000U
#define KERNEL_TOP 0xFFFFFFFFU

#define PA_TO_VA(pa) ((void*)(uintptr_t)(pa) + KERNEL_BASE)

#define VA_TO_PA(va) ((uintptr_t)(va) - KERNEL_BASE)

#define PAGE_ALIGN_DOWN(addr) (((uintptr_t)(addr)) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(addr) \
  (((uintptr_t)(addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

/* Extract page table indexes (32-bit x86 two-level paging) */

#define PDE_INDEX(va) (((uintptr_t)(va) >> 22) & 0x3FF)

#define PTE_INDEX(va) (((uintptr_t)(va) >> 12) & 0x3FF)

#define PAGE_OFFSET(va) ((uintptr_t)(va) & 0xFFF)

#define PTE_ADDR_MASK 0xFFFFF000U
#define PTE_TO_PA(pte) ((pte) & PTE_ADDR_MASK)

#define MAKE_PTE(pa, flags) (((uint32_t)(pa) & PTE_ADDR_MASK) | (flags))
#define MAKE_PDE(pt_pa, flags) (((uint32_t)(pt_pa) & PTE_ADDR_MASK) | (flags))
#endif
