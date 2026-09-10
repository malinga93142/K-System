#include <stdint.h>
#include "multiboot.h"
#include "../mm/pmm.h"
#include <stddef.h>
void parse_mmap(struct multiboot_info *mbi, struct phys_range usable[32],
                uint8_t *howmany, uint8_t usable_idxs[32])
{
  uint8_t *ptr = mbi->tags;
  uint8_t *end = (uint8_t *)mbi + mbi->total_size;
  int i = 0;

  while (ptr < end)
  {
    struct multiboot_tag *tag = (struct multiboot_tag *)ptr;

    if (tag->type == 0)
      break;

    if (tag->type == 6)
    { /* MEMORY_MAP */
      struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)tag;

      uint8_t *entry_ptr = (uint8_t *)mmap->entries;

      while (entry_ptr < ptr + tag->size)
      {
        struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry *)entry_ptr;

        if (e->type == 1 && i < 32)
        {
          usable[i].base = e->addr;
          usable[i].len = e->len;
          usable_idxs[i] = 1;
          i++;
        }

        entry_ptr += mmap->entry_size;
      }
    }

    ptr += (tag->size + 7) & ~7;
  }

  *howmany = i;
}

#define MAX_MODULES 8

static struct
{
  uint32_t start;
  uint32_t end;
} loaded_modules[MAX_MODULES];

static uint32_t loaded_module_count = 0;

void parse_modules(struct multiboot_info *mbi)
{
  loaded_module_count = 0;

  uint8_t *ptr = mbi->tags;
  uint8_t *end = (uint8_t *)mbi + mbi->total_size;
  while (ptr < end)
  {
    struct multiboot_tag *tag = (struct multiboot_tag *)ptr;

    if (tag->type == 0)
      break;

    if (tag->type == 3)
    { /* MODULE */
      if (loaded_module_count < MAX_MODULES)
      {
        struct multiboot_tag_module *mod = (struct multiboot_tag_module *)tag;

        loaded_modules[loaded_module_count].start = mod->mod_start;

        loaded_modules[loaded_module_count].end = mod->mod_end;

        pmm_reserve_region(mod->mod_start,
                           mod->mod_end - mod->mod_start);

        loaded_module_count++;
      }
    }

    ptr += (tag->size + 7) & ~7;
  }
}
