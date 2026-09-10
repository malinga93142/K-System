// #ifndef PAGING_H
// #define PAGING_H
// #include <stdint.h>
// /*
// 63             52 51        12 11        9 8 7 6 5 4   3   2 1 0
// +----------------+------------+-----------+-+--+-+-+---+---+-+---+
// | NX             | Phys Addr  | Software  |G|PS|D|A|PCD|PWT|U|W|P|
// +----------------+------------+-----------+-+--+-+-+---+---+-+---+
// */
// #define PTE_P      (1ULL << 0)   // Present
// #define PTE_RW     (1ULL << 1)   // Read/Write
// #define PTE_US     (1ULL << 2)   // User/Supervisor
// #define PTE_PWT    (1ULL << 3)   // Page Write Through
// #define PTE_PCD    (1ULL << 4)   // Page Cache Disable
// #define PTE_A      (1ULL << 5)   // Accessed
// #define PTE_D      (1ULL << 6)   // Dirty
// #define PTE_PS     (1ULL << 7)   // Page Size (large page)
// #define PTE_G      (1ULL << 8)   // Global
// #define PTE_NX     (1ULL << 63)  // No Execute

// #define PDE_P			PTE_P
// #define PDE_RW		PTE_RW
// #define PDE_US		PTE_US

// void init_paging();
// void map_page(uint32_t va, uint32_t pa, uint32_t flags);
// #endif

#ifndef PAGING_H
#define PAGING_H

#include "mm.h"
#include <stdint.h>
// #define PAGE_SIZE        0x1000
#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_PCD	0x10
#define PD_RECURSIVE_INDEX 1023
#define RECURSIVE_PD_VADDR 0xFFFFF000u
#define RECURSIVE_PT_VADDR(pd_idx) (0xFFC00000u + ((pd_idx) << 12))


// #define PDE_INDEX(v)  (((uint32_t)(v)) >> 22)
// #define PTE_INDEX(v)  ((((uint32_t)(v)) >> 12) & 0x3FF)
// #define PAGE_ALIGN_DOWN(x) ((x) & ~(PAGE_SIZE - 1))
// #define PAGE_ALIGN_UP(x)   (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

void vmm_init(uint32_t kernel_phys_start, uint32_t kernel_phys_end);
uint32_t *vmm_new_directory(void);
int vmm_map_page(uint32_t *pd, uint32_t vaddr, uint32_t paddr, uint32_t flags);
void vmm_switch_directory(uint32_t *pd_phys);
uint32_t vmm_get_phys(uint32_t *pd, uint32_t vaddr);
int vmm_map_page_post_switch(uint32_t vaddr, uint32_t paddr, uint32_t flags);
int vmm_map_fresh_page(uint32_t vaddr, uint32_t paddr, uint32_t flags);
int uvm_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags);
int uvm_load_page(uint32_t vaddr,const void *src,uint32_t size);
int uvm_create_stack(uint32_t s_top, uint32_t size);
uint32_t kvm_create_guarded_stack(uint32_t top_vaddr, uint32_t size);
#endif
