#include "lapic.h"

#include "../drivers/uart.h"
#include "../mm/paging.h"
#include "../mm/pmm.h"

#define LAPIC_VIRT_ADDR 0xC0500000

#define LAPIC_REG_ID 0x020
#define LAPIC_REG_SPURIOUS 0x0F0
#define LAPIC_REG_EOI 0x0B0
#define LAPIC_REG_ICR_LOW 0x300
#define LAPIC_REG_ICR_HIGH 0x310
#define LAPIC_REG_TIMER_LVT 0x320
#define LAPIC_REG_TIMER_INITCNT 0x380
#define LAPIC_REG_TIMER_CURCNT 0x390
#define LAPIC_REG_TIMER_DIV 0x3E0

static volatile uint32_t* lapic = (volatile uint32_t*)LAPIC_VIRT_ADDR;

uint32_t lapic_read(uint32_t reg) { return lapic[reg / 4]; }

void lapic_write(uint32_t reg, uint32_t val) {
  lapic[reg / 4] = val;
}

static int has_lapic(void) {
  uint32_t eax, edx;
  __asm__ volatile("cpuid" : "=a"(eax), "=d"(edx) : "a"(1) : "ebx", "ecx");
  return (edx & (1 << 9)) != 0;
}

static uint32_t get_lapic_base_phys(void) {
  uint32_t lo, hi;
  __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1B));
  (void)hi; /* bits above 32 only matter on systems with >4GB phys addressing;
               ignored here since we're 32-bit */
  return lo & 0xFFFFF000u;
}

void lapic_init(void) {
  if (!has_lapic()) {
    uart_puts("lapic: CPUID reports no LAPIC present, halting\n");
    for (;;) __asm__ volatile("hlt");
  }

  uint32_t lapic_phys = get_lapic_base_phys();
  uart_puts("lapic: physical base=");
  uart_hex(lapic_phys);
  uart_puts("\n");

  /* defensively reserve, even though e820 should already exclude this
     range -- protects against pmm ever handing this frame to something
     else if the memory map ever reports it as usable */
  pmm_reserve_region(lapic_phys, PAGE_SIZE);

  /* map with cache-disable -- required for MMIO correctness; a cached
     read/write here would not reliably reach real hardware registers */
  vmm_map_page_post_switch(LAPIC_VIRT_ADDR, lapic_phys, PAGE_RW | PAGE_PCD);

  /* software-enable the LAPIC, spurious vector = 0xFF (conventional) */
  lapic_write(LAPIC_REG_SPURIOUS,
              lapic_read(LAPIC_REG_SPURIOUS) | 0x100 | 0xFF);

  uint32_t id = lapic_get_id();
  uart_puts("lapic: enabled, id=");
  uart_hex(id);
  uart_puts("\n");
}

uint32_t lapic_get_id(void) { return lapic_read(LAPIC_REG_ID) >> 24; }

void lapic_send_eoi(void) { lapic_write(LAPIC_REG_EOI, 0); }
