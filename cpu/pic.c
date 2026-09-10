#include "pic.h"

#include "../io/io.h"
static void io_wait(void) {
  /*
   * Old x86 PICs sometimes need a small delay
   * between commands.
   */
  __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
  /*
   * Save current interrupt masks.
   */
  uint8_t mask1 = inb(PIC1_DATA);
  uint8_t mask2 = inb(PIC2_DATA);

  /*
   * ICW1:
   * 0x11 = initialize PIC + expect ICW4
   */
  outb(PIC1_COMMAND, 0x11);
  io_wait();

  outb(PIC2_COMMAND, 0x11);
  io_wait();

  /*
   * ICW2:
   * Set interrupt vector offsets.
   */
  outb(PIC1_DATA, offset1);
  io_wait();

  outb(PIC2_DATA, offset2);
  io_wait();

  /*
   * ICW3:
   *
   * Master: slave is connected to IRQ2.
   * Slave: identity = 2.
   */
  outb(PIC1_DATA, 0x04);
  io_wait();

  outb(PIC2_DATA, 0x02);
  io_wait();

  /*
   * ICW4:
   * 0x01 = 8086/88 mode.
   */
  outb(PIC1_DATA, 0x01);
  io_wait();

  outb(PIC2_DATA, 0x01);
  io_wait();

  /*
   * Restore the previous interrupt masks.
   */
  outb(PIC1_DATA, mask1);
  outb(PIC2_DATA, mask2);
}

void pic_mask_all(void) {
  outb(PIC1_DATA, 0xFF);
  outb(PIC2_DATA, 0xFF);
}

void pic_unmask_irq(uint8_t irq) {
  if (irq < 8) {
    uint8_t mask = inb(PIC1_DATA);
    mask &= ~(1 << irq);
    outb(PIC1_DATA, mask);
  } else if (irq < 16) {
    /*
     * IRQ 8-15 live on the slave PIC.
     *
     * Also unmask IRQ2 on the master because
     * IRQ2 is the cascade connection to the slave.
     */
    uint8_t mask2 = inb(PIC2_DATA);
    mask2 &= ~(1 << (irq - 8));
    outb(PIC2_DATA, mask2);

    uint8_t mask1 = inb(PIC1_DATA);
    mask1 &= ~(1 << 2);
    outb(PIC1_DATA, mask1);
  }
}
void pic_mask_irq(uint8_t irq) {
  uint16_t port = (irq < 8) ? 0x21 : 0xA1; /* master vs slave data port */
  uint8_t irq_line = (irq < 8) ? irq : irq - 8;
  uint8_t mask = inb(port);
  mask |= (1 << irq_line);
  outb(port, mask);
}
void pic_send_eoi(uint8_t irq) {
  if (irq >= 8) {
    outb(PIC2_COMMAND, PIC_EOI);
  }

  outb(PIC1_COMMAND, PIC_EOI);
}
void pit_init(uint32_t frequency) {
  uint32_t divisor = 1193182 / frequency;

  outb(PIT_COMMAND, 0x36);

  outb(PIT_CHANNEL0, divisor & 0xFF);
  outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}
