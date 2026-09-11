#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#include "mm.h"
/* parses the multiboot mmap into __phys_e820_info (defined in multiboot.c) */
void read_mmapable_range(uint32_t mbi);

/* --- PMM --- */
// uint32_t pmm_alloc_frame_below(uint32_t);
uint32_t pmm_alloc_frame_below(uint32_t range);
/* Call once at boot, after read_mmapable_range has populated __phys_e820_info.
 * Builds the frame bitmap: everything defaults to "used", then usable
 * e820 regions are punched out as free. */
void pmm_init(uint32_t mbi);

/* Mark [base, base+len) as used (rounds outward to whole frames).
 * Use this to reserve the kernel image, the multiboot info struct,
 * the mmap buffer itself, etc. before handing out frames. */
void pmm_reserve_region(uint32_t base, uint32_t len);

/* Returns the physical address of a free 4KB frame and marks it used,
 * or 0 if out of memory. 0 is never a valid returned frame because
 * frame 0 is always reserved (see pmm_init). */
uint32_t pmm_alloc_frame(void);

/* Marks a previously-allocated frame free again. addr must be
 * page-aligned. */
void pmm_free_frame(uint32_t addr);

/* Stats, mainly for debugging via uart */
uint32_t pmm_total_frames(void);
uint32_t pmm_free_frames(void);

#endif
