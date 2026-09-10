#include "gdt.h"
#include "../mm/paging.h"
#define MAX_CPU				4
#define KERNEL_ISTACK_TOP 0xc0200000
#define KERNEL_ISTACK_SIZE (16 * 1024)
struct gdt_entry gdt32[6]; /* null, kcode, kdata, ucode, udata, tss */
struct tss_entry tss;
struct gdtr gdtptr;


struct gdt_entry gdt32_percpu[MAX_CPU][6];
struct tss_entry tss_percpu[MAX_CPU];
#if defined(AVL_SMP) && AVL_SMP == 4
static void init_tss_for_cpu(int cpu_id)
#else
static void init_tss(void)
#endif
{
#if defined(AVL_SMP) && AVL_SMP == 4
	tss_percpu[cpu_id] = (struct tss_entry){0};
	tss_percpu[cpu_id].ss = 0x10;
	uint32_t top = kvm_create_guarded_stack(0xc0a00000 + cpu_id * 0x1000, 0x4000);
	tss_percpu[cpu_id].esp0=top;
	tss_percpu[cpu_id].iomap_base = sizeof(struct tss_entry);
#else
  uint32_t istack_top =
      kvm_create_guarded_stack(KERNEL_ISTACK_TOP, KERNEL_ISTACK_SIZE);
  tss = (struct tss_entry){0};
  tss.ss0 = 0x10; /* kernel data selector */
  tss.esp0 = istack_top;
  tss.iomap_base =
      sizeof(tss); /* no I/O bitmap, so this points past the struct */
#endif
}
#if defined(AVL_SMP) && AVL_SMP==4
static void set_tss_descriptor(struct gdt_entry *g, uint32_t base, uint32_t limit) {
	g->limit_low = limit & 0xffff;
	g->base_low	= base & 0xffff;
	g->base_mid = (base >> 16) & 0xff;
	g->base_high= (base >> 24) & 0xff;
	g->gran = (limit >> 16) & 0x0f;
	g->access=0x89;
#else
static void set_tss_descriptor(uint32_t index, uint32_t base, uint32_t limit) {
  gdt32[index].limit_low = limit & 0xFFFF;
  gdt32[index].base_low = base & 0xFFFF;
  gdt32[index].base_mid = (base >> 16) & 0xFF;
  gdt32[index].base_high = (base >> 24) & 0xFF;
  gdt32[index].gran = (limit >> 16) & 0x0F;
  gdt32[index].access = 0x89; /* present, ring0, 32-bit TSS (available) */
#endif
}

void init_gdt(int cpu_id) {
#if defined(AVL_SMP) && AVL_SMP==4
	struct gdt_entry *g =  gdt32_percpu[cpu_id];
	g[0]=(struct gdt_entry){0};
	g[1]=(struct gdt_entry){0}; g[1].limit_low = 0xffff; g[1].gran=0xcf; g[1].access=0x9a;
	g[2]=(struct gdt_entry){0}; g[2].limit_low = 0xffff; g[2].gran=0xcf; g[2].access=0x92;
	g[3]=(struct gdt_entry){0}; g[3].limit_low = 0xffff; g[3].gran=0xcf; g[3].access=0xFA;
	g[4]=(struct gdt_entry){0}; g[4].limit_low = 0xffff; g[4].gran=0xcf; g[4].access=0xF2;
	init_tss_for_cpu(cpu_id);
	set_tss_descriptor(&g[5], (uint32_t)(uintptr_t)&tss_percpu[cpu_id], sizeof(struct tss_entry)-1);
	gdtptr.base = (uint32_t)(uintptr_t)&gdt32_percpu[cpu_id];
	gdtptr.limit = sizeof(gdt32_percpu[cpu_id]) - 1;
#else
	(void)cpu_id;
  gdt32[0] = (struct gdt_entry){0}; /* null */
  /* index 1: kernel code, ring0 */
  gdt32[1] = (struct gdt_entry){0};
  gdt32[1].limit_low = 0xFFFF;
  gdt32[1].gran = 0xCF;
  gdt32[1].access = 0x9A;
  /* index 2: kernel data, ring0 */
  gdt32[2] = (struct gdt_entry){0};
  gdt32[2].limit_low = 0xFFFF;
  gdt32[2].gran = 0xCF;
  gdt32[2].access = 0x92;
  /* index 3: user code, ring3 - DPL bits (5,6) = 11 in access byte */
  gdt32[3] = (struct gdt_entry){0};
  gdt32[3].limit_low = 0xFFFF;
  gdt32[3].gran = 0xCF;
  gdt32[3].access = 0xFA; /* present, DPL=3, code, exec/read */
  /* index 4: user data, ring3 */
  gdt32[4] = (struct gdt_entry){0};
  gdt32[4].limit_low = 0xFFFF;
  gdt32[4].gran = 0xCF;
  gdt32[4].access = 0xF2; /* present, DPL=3, data, read/write */
  /* index 5: TSS */
  init_tss();
  set_tss_descriptor(5, (uint32_t)(uintptr_t)&tss, sizeof(tss) - 1);
  gdtptr.base = (uint32_t)(uintptr_t)&gdt32;
  gdtptr.limit = sizeof(gdt32) - 1;
#endif
  asm volatile("lgdt %0" ::"m"(gdtptr) : "memory");

  asm volatile(
      "mov $0x10, %%ax\n\t"
      "mov %%ax, %%ds\n\t"
      "mov %%ax, %%es\n\t"
      "mov %%ax, %%fs\n\t"
      "mov %%ax, %%gs\n\t"
      "mov %%ax, %%ss\n\t"
      "ljmp $0x08, $1f\n\t"
      "1:\n\t"
      "mov $0x2B, %%ax\n\t" /* TSS selector: index 5 * 8 = 0x28, | RPL
                               3 = 0x2B */
      "ltr %%ax\n\t" ::
          : "ax");

}
