#include "../boot/multiboot.h"
#include "../cpu/spinlock.h"
#include "../drivers/uart.h"
extern char kernel_start[];
extern char kernel_end[];

/* Worst case for a 32-bit PA system: 4GB / 4KB = 1,048,576 frames.
 * 1 bit per frame -> 131072 bytes (128 KiB) static bitmap.
 * bit = 1 -> used/reserved, bit = 0 -> free. */
#define TOTAL_FRAMES (0xFFFFFFFFu / PAGE_SIZE + 1u)
#define BITMAP_BYTES (TOTAL_FRAMES / 8u)
static spinlock_t pmm_lock;
static uint8_t bitmap[BITMAP_BYTES];
static uint32_t free_count = 0;
static uint32_t last_alloc_frame =
    0; /* next-fit hint, avoids rescanning from 0 every time */

static inline void bit_set(uint32_t frame) {
  bitmap[frame >> 3] |= (uint8_t)(1u << (frame & 7));
}
static inline void bit_clear(uint32_t frame) {
  bitmap[frame >> 3] &= (uint8_t)~(1u << (frame & 7));
}
static inline int bit_test(uint32_t frame) {
  return bitmap[frame >> 3] & (uint8_t)(1u << (frame & 7));
}
/* --- PMM --- */
void pmm_reserve_region(uint32_t base, uint32_t len) {
  if (len == 0) return;
  uint32_t start_frame = base / PAGE_SIZE;
  uint32_t end = base + len;
  uint32_t end_frame = (end + PAGE_SIZE - 1) / PAGE_SIZE; /* round up */
  if (end_frame > TOTAL_FRAMES) end_frame = TOTAL_FRAMES;
  for (uint32_t f = start_frame; f < end_frame; f++) {
    if (!bit_test(f)) {
      bit_set(f);
      free_count--;
    }
  }
}
static void mark_region_free(uint32_t base, uint32_t len) {
  if (len == 0) return;
  /* round base UP and end DOWN so we never mark a partial frame at
   * either edge as free (safer to under-report than to hand out a
   * frame that's half outside the usable region) */
  uint32_t start_frame = (base + PAGE_SIZE - 1) / PAGE_SIZE;
  uint32_t end = base + len;
  uint32_t end_frame = end / PAGE_SIZE;
  if (end_frame > TOTAL_FRAMES) end_frame = TOTAL_FRAMES;
  for (uint32_t f = start_frame; f < end_frame; f++) {
    if (bit_test(f)) {
      bit_clear(f);
      free_count++;
    }
  }
}
uint32_t pmm_alloc_frame_below(uint32_t limit) {
  if (free_count == 0) {
    return 0;
  }
  uint32_t max_frame = limit / PAGE_SIZE;
  if (max_frame > TOTAL_FRAMES) max_frame = TOTAL_FRAMES;
  for (uint32_t f = 0; f < max_frame; f++) {
    if (!bit_test(f)) {
      bit_set(f);
      free_count--;
      return f * PAGE_SIZE;
    }
  }
  uart_puts("pmm: BUG or OOM in pmm_alloc_frame_below, limit=");
  uart_hex(limit);
  uart_puts("\n");
  return 0;
}
void pmm_init(uint32_t mbi) {
  for (uint32_t i = 0; i < BITMAP_BYTES; i++) bitmap[i] = 0xFF;
  free_count = 0;
  struct phys_range range[32];
  uint8_t count;
  uint8_t idx[32];
  parse_mmap((struct multiboot_info*)mbi, range, &count, idx);
  /* Deliberately skip the first e820 usable region (typically 0x0-0x9FC00,
   conventional low memory). Real firmware sometimes leaves BIOS Data
   Area / EBDA structures there that aren't safe to blindly reuse, even
   when e820 marks the region as type-1 usable. Low memory is also not
   needed for anything we do right now (AP trampoline will need a small,
   specific chunk of it later -- see TODO -- at which point we'll
   explicitly carve out and reserve just that address, rather than
   opening the whole region to the general allocator). */
  for (uint8_t i = 1; i < count; i++) {
    mark_region_free(range[i].base, range[i].len);
  }
  /* frame 0 (physical address 0) is always reserved, even if it fell
   * inside a usable region -- so pmm_alloc_frame() can use 0 as an
   * unambiguous "out of memory" sentinel. */
  pmm_reserve_region(0, PAGE_SIZE);
  last_alloc_frame = 0;
  uart_puts("pmm: init done, free frames: ");
  uart_dec(free_count);
  uart_puts(" (");
  uart_dec(free_count * (PAGE_SIZE / 1024));
  uart_puts(" KB)\n");
  uint32_t k_phys_start = (uint32_t)(uintptr_t)kernel_start - KERNEL_BASE;
  uint32_t k_phys_end = (uint32_t)(uintptr_t)kernel_end - KERNEL_BASE;
  pmm_reserve_region(k_phys_start, k_phys_end - k_phys_start);
  spinlock_init(&pmm_lock);
}
uint32_t pmm_alloc_frame(void) {
  uint32_t eflags = disable_local_interrupts();
  spinlock_acquire(&pmm_lock);
  uint32_t result = 0;
  // uart_puts("lock acquired\n");
  if (free_count == 0) goto out;
  /* next-fit: start scanning where we left off, wrap around once */
  for (uint32_t f = last_alloc_frame; f < TOTAL_FRAMES; f++) {
    if (!bit_test(f)) {
      bit_set(f);
      free_count--;
      last_alloc_frame = f + 1;
      result = f * PAGE_SIZE;
      goto out;
    }
  }
  for (uint32_t f = 0; f < last_alloc_frame; f++) {
    if (!bit_test(f)) {
      bit_set(f);
      free_count--;
      last_alloc_frame = f + 1;
      result = f * PAGE_SIZE;
      goto out;
    }
  }
  /* free_count said we had frames but the scan found none — bitmap
   * bookkeeping bug, don't silently hand out garbage */
  uart_puts("pmm: BUG free_count/bitmap mismatch\n");
out:
  spinlock_release(&pmm_lock);
  restore_local_interrupts(eflags);
  return result;
}
void pmm_free_frame(uint32_t addr) {
  uint32_t eflags = disable_local_interrupts();
  spinlock_acquire(&pmm_lock);
  if (addr % PAGE_SIZE != 0) {
    uart_puts("pmm: BUG free_frame given unaligned address\n");
    goto out;
  }
  uint32_t frame = addr / PAGE_SIZE;
  if (frame == 0 ||
      frame >= TOTAL_FRAMES) { /* refuse to free the sentinel/out-of-range */
    goto out;
  }
  if (bit_test(frame)) {
    bit_clear(frame);
    free_count++;
  }
  /* freeing an already-free frame is silently ignored rather than
   * treated as an error -- double-free detection would need a
   * refcount, which is more than "simple" calls for right now */
out:
  restore_local_interrupts(eflags);
  spinlock_release(&pmm_lock);
}
uint32_t pmm_total_frames(void) { return TOTAL_FRAMES; }
uint32_t pmm_free_frames(void) { return free_count; }
