#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "../mm/pmm.h"
#include <stdint.h>

struct multiboot_tag
{
  uint32_t type;
  uint32_t size;
};
struct multiboot_mmap_entry
{
  uint64_t addr;
  uint64_t len;
  uint32_t type;
  uint32_t zero;
} __attribute__((packed));
struct multiboot_tag_mmap
{
  uint32_t type;
  uint32_t size;

  uint32_t entry_size;
  uint32_t entry_version;

  struct multiboot_mmap_entry entries[];
};

struct multiboot_mmap_entry
{
  uint64_t addr;
  uint64_t len;
  uint32_t type;
  uint32_t zero;
} __attribute__((packed));

struct multiboot_tag_module
{
  uint32_t type;
  uint32_t size;

  uint32_t mod_start;
  uint32_t mod_end;

  char string[];
};

struct multiboot_info
{
  uint32_t total_size;
  uint32_t reserved;
  uint8_t tags[];
};
struct phys_range
{
  uint32_t base;
  uint32_t len;
};
void parse_mmap(struct multiboot_info *mbi, struct phys_range usable[32],
                uint8_t *howmany, uint8_t usable_idxs[32]);

void parse_modules(struct multiboot_info *mbi);

#endif
