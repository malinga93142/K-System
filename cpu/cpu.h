#ifndef CPU_H
#define CPU_H
#include "gdt.h"
#include "idt.h"

#define MAX_CPUS 2
struct cpu {
  uint8_t lapic_id;
  uint8_t cpu_id;
  uint8_t started;
  volatile uint32_t ncli;
  uint32_t intena;
  struct gdt_entry *gdt;
  struct tss_entry *tss;
};
extern struct cpu cpus[MAX_CPUS];

struct cpu *mycpu(void);
uint8_t mycpuid(void);
void initcpus(void);
#endif
