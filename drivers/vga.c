#include "vga.h"

static int vga_col = 0;
static int vga_row = 0;

void vga_clear(void)
{
  for (int r = 0; r < VGA_ROWS; r++)
  {
    for (int c = 0; c < VGA_COLS; c++)
    {
      VGA_BUFFER[r * VGA_COLS + c] = ((uint16_t)0x0F << 8) | (uint8_t)' ';
    }
  }
  vga_col = 0;
  vga_row = 0;
}

static void __vga_putc(char c)
{
  if (c == '\n')
  {
    vga_col = 0;
    vga_row++;
    if (vga_row >= VGA_ROWS)
      vga_row = 0; /* wrap first, before writing prompt */

    /* print "$>" at the start of the new line */
    const char *prompt = "$";
    for (int i = 0; prompt[i]; i++)
    {
      VGA_BUFFER[vga_row * VGA_COLS + vga_col] =
          ((uint16_t)0x0F << 8) | (uint8_t)prompt[i];
      vga_col++;
    }
  }
  else
  {
    VGA_BUFFER[vga_row * VGA_COLS + vga_col] =
        ((uint16_t)0x0F << 8) | (uint8_t)c;
    vga_col++;
    if (vga_col >= VGA_COLS)
    {
      vga_col = 0;
      vga_row++;
      if (vga_row >= VGA_ROWS)
        vga_row = 0;
    }
  }
}

void vga_putc(char c) { __vga_putc(c); }
void vga_puts(const char *c)
{
  while (*c)
  {
    vga_putc(*c);
    c++;
  }
}
