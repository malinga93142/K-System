#include "paging.h"

#include "../drivers/uart.h"
#include "../kernel/kstring.h"
#include "../cpu/spinlock.h"
#include "pmm.h"
#define KERNEL_VIRT_BASE 0xC0000000


static spinlock_t paging_lock;
static inline uint32_t* phys_to_temp_va(uint32_t paddr) {
  return (uint32_t*)(paddr + KERNEL_VIRT_BASE);
}
uint32_t* vmm_new_directory(void) {
  uint32_t pd_phys = pmm_alloc_frame();
  uint32_t* pd = (uint32_t*)PA_TO_VA(pd_phys); /*phys_to_temp_va(pd_phys);*/
  if (pd == 0) {
    uart_puts("vmm: failed to allocate page directory below 4MB\n");
    for (;;) asm volatile("hlt");
  }
  for (int i = 0; i < 1024; i++) pd[i] = 0;
  pd[PD_RECURSIVE_INDEX] = (pd_phys & ~0xFFF) | PAGE_PRESENT | PAGE_RW;
  return (uint32_t*)pd_phys; /* caller stores/treats this as physical */
}

static uint32_t* get_or_alloc_pagetable(uint32_t* pd_phys, uint32_t vaddr) {
  uint32_t eflags = disable_local_interrupts();
  spinlock_acquire(&paging_lock);
  uint32_t* pd = (uint32_t*)PA_TO_VA((uint32_t)pd_phys);
  uint32_t idx = PDE_INDEX(vaddr);
  if (!(pd[idx] & PAGE_PRESENT)) {
    uint32_t pt_phys = pmm_alloc_frame();
    uint32_t* pt = (uint32_t*)PA_TO_VA(pt_phys);
    for (int i = 0; i < 1024; i++) pt[i] = 0;
    pd[idx] = (pt_phys & ~0xFFF) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
  }
  spinlock_release(&paging_lock);
  restore_local_interrupts(eflags);
  return (uint32_t*)PA_TO_VA(pd[idx] & ~0xFFF);
}
int vmm_map_page(uint32_t* pd_phys, uint32_t vaddr, uint32_t paddr,
                 uint32_t flags) {
  uint32_t* pt = get_or_alloc_pagetable(pd_phys, vaddr);
  uint32_t idx = PTE_INDEX(vaddr);
  pt[idx] = (paddr & ~0xFFF) | (flags & 0xFFF) | PAGE_PRESENT;
  return 0;
}

uint32_t vmm_get_phys(uint32_t* pd_phys, uint32_t vaddr) {
  uint32_t* pd = (uint32_t*)PA_TO_VA((uint32_t)pd_phys);
  uint32_t idx = PDE_INDEX(vaddr);
  if (!(pd[idx] & PAGE_PRESENT)) return 0;
  uint32_t* pt = (uint32_t*)PA_TO_VA(pd[idx] & ~0xFFF);
  uint32_t pte = pt[PTE_INDEX(vaddr)];
  if (!(pte & PAGE_PRESENT)) return 0;
  return (pte & ~0xFFF) | (vaddr & 0xFFF);
}

void vmm_switch_directory(uint32_t* pd_phys) {
  __asm__ volatile("mov %0, %%cr3" ::"r"(pd_phys) : "memory");
}

void vmm_init(uint32_t kernel_phys_start, uint32_t kernel_phys_end) {
  uint32_t* pd_phys = vmm_new_directory();
  for (uint32_t p = PAGE_ALIGN_DOWN(kernel_phys_start);
       p < PAGE_ALIGN_UP(kernel_phys_end); p += PAGE_SIZE) {
    vmm_map_page(pd_phys, p + KERNEL_VIRT_BASE, p, PAGE_RW);
  }
  vmm_map_page(pd_phys, 0xC00B8000, 0xB8000, PAGE_RW);
  vmm_switch_directory(pd_phys);
}

static uint32_t* get_or_alloc_pagetable_post_switch(uint32_t vaddr,
                                                    uint32_t flags) {
  // uint32_t eflags = disable_local_interrupts();
  // spinlock_acquire(&paging_lock);
  uint32_t pd_idx = PDE_INDEX(vaddr);
  uint32_t* pd = (uint32_t*)RECURSIVE_PD_VADDR;
  if (!(pd[pd_idx] & PAGE_PRESENT)) {
    uint32_t pt_phys = pmm_alloc_frame();
    if (pt_phys == 0) {
      uart_puts("vmm: OOM allocating PT\n");
      // spinlock_release(&paging_lock);
      // restore_local_interrupts(eflags);
      for (;;) __asm__ volatile("hlt");
    }

    uint32_t pde_flags = PAGE_PRESENT | PAGE_RW;
    if (flags & PAGE_USER) pde_flags |= PAGE_USER;

    pd[pd_idx] = (pt_phys & ~0xFFF) | pde_flags;
    __asm__ volatile("invlpg (%0)" ::"r"(RECURSIVE_PT_VADDR(pd_idx))
                     : "memory");

    uint32_t* pt = (uint32_t*)(uintptr_t)RECURSIVE_PT_VADDR(pd_idx);
    for (int i = 0; i < 1024; i++) pt[i] = 0;
  } else if (flags & PAGE_USER) {
    pd[pd_idx] |= PAGE_USER;
  }
  // spinlock_release(&paging_lock);
  // restore_local_interrupts(eflags);
  return (uint32_t*)(uintptr_t)RECURSIVE_PT_VADDR(pd_idx);
}

int vmm_map_page_post_switch(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
  uint32_t eflags = disable_local_interrupts();
  spinlock_acquire(&paging_lock);
  uint32_t* pt = get_or_alloc_pagetable_post_switch(vaddr, flags);
  pt[PTE_INDEX(vaddr)] = (paddr & ~0xFFF) | (flags & 0xFFF) | PAGE_PRESENT;
  __asm__ volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
  // memset((void *)(vaddr & ~0xFFF), 0, PAGE_SIZE);
  spinlock_release(&paging_lock);
  restore_local_interrupts(eflags);
  return 0;
}
int vmm_map_fresh_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
  int rc = vmm_map_page_post_switch(vaddr, paddr, flags);
  if (rc == 0) memset((void*)(vaddr & ~0xFFF), 0, PAGE_SIZE);
  return rc;
}

int uvm_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
  return vmm_map_page_post_switch(vaddr, paddr, flags);
}

int uvm_load_page(uint32_t vaddr, const void* src, uint32_t size) {
  uint32_t npages = PAGE_ALIGN_UP(size) / PAGE_SIZE;
  for (uint32_t i = 0; i < npages; i++) {
    uint32_t page_vaddr = vaddr + i * PAGE_SIZE;
    uint32_t frame = pmm_alloc_frame();
    if (frame == 0) {
      uart_puts("uvm_load_code: OOM\n");
      return -1;
    }
    if (uvm_map_page(page_vaddr, frame, USER_CODE_FLAGS) != 0) {
      pmm_free_frame(frame);
      uart_puts("uvm_load_code: map failed\n");
      return -1;
    }
  }
  memcpy((void*)vaddr, src, size);

  uint32_t total_mapped = npages * PAGE_SIZE;
  if (total_mapped > size)
    memset((void*)(vaddr + size), 0, total_mapped - size);
  return 0;
}

int uvm_create_stack(uint32_t stop_address, uint32_t size) {
  uint32_t npages = PAGE_ALIGN_UP(size) / PAGE_SIZE;
  uint32_t sbottom_vaddr = stop_address - npages * PAGE_SIZE;
  for (uint32_t i = 0; i < npages; i++) {
    uint32_t page_vaddr = sbottom_vaddr + i * PAGE_SIZE;
    uint32_t frame = pmm_alloc_frame();
    if (frame == 0) {
      uart_puts("uvm_create_stack: OOM\n");
      return -1;
    }
    if (uvm_map_page(page_vaddr, frame, USER_STACK_FLAGS) != 0) {
      pmm_free_frame(frame);
      uart_puts("uvm_create_stack: map failed\n");
      return -1;
    }
  }
  return 0;
}
uint32_t kvm_create_guarded_stack(uint32_t top_vaddr, uint32_t size) {
  uint32_t npages = PAGE_ALIGN_UP(size) / PAGE_SIZE;
  uint32_t bottom_vaddr = top_vaddr - npages * PAGE_SIZE;
  for (uint32_t i = 0; i < npages; i++) {
    uint32_t page_vaddr = bottom_vaddr + i * PAGE_SIZE;
    uint32_t frame = pmm_alloc_frame();
    if (frame == 0) {
      uart_puts("uvm_create_stack: OOM\n");
      for (;;) asm volatile("hlt");
    }
    vmm_map_page_post_switch(page_vaddr, frame, PAGE_RW);
  }
  return top_vaddr;
}