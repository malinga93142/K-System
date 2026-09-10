#include "keyboard.h"

#include "../cpu/pic.h"
#include "../cpu/spinlock.h"
#include "../io/io.h"
#include "uart.h"

#define KBD_DATA_PORT 0x60

/*
 * US QWERTY scancode set 1, make-codes only (key press).
 * Index = scancode, value = ASCII, 0 = unmapped/no direct ASCII
 * (shift, ctrl, function keys, etc.  not handled yet.
 */
static const char
    scancode_ascii[128] =
        {
            0,    27,  '1', '2', '3',  '4',  '5', '6', '7',  '8',
            '9',  '0', '-', '=', '\b', '\t', 'q', 'w', 'e',  'r',
            't',  'y', 'u', 'i', 'o',  'p',  '[', ']', '\n', 0, /* left ctrl */
            'a',  's', 'd', 'f', 'g',  'h',  'j', 'k', 'l',  ';',
            '\'', '`', 0, /* left shift */
            '\\', 'z', 'x', 'c', 'v',  'b',  'n', 'm', ',',  '.',
            '/',  0, /* right shift */
            '*',  0, /* alt */
            ' ',     /* space */
            0,       /* caps lock */
                     /* function keys, numpad, etc. all 0 for now */
};

#define KBD_BUF_SIZE 256
static char kbd_buf[KBD_BUF_SIZE];
static volatile uint32_t kbd_head = 0;
static volatile uint32_t kbd_tail = 0;
static spinlock_t kbd_lock;

void keyboard_init(void) {
  spinlock_init(&kbd_lock);
  kbd_head = kbd_tail = 0;
}

void keyboard_handler(struct exception_frame* ef) {
  (void)ef;
  uint8_t scancode = inb(KBD_DATA_PORT);

  /* bit 7 set = key RELEASE (break code) -- ignore for now, we only
     handle key press (make codes) */
  if (scancode & 0x80) {
    return;
  }

  if (scancode >= 128)
    return; /* defensive bound, shouldn't happen given the mask above */

  char c = scancode_ascii[scancode];
  if (c == 0) return; /* unmapped key, ignore */

  spinlock_acquire(&kbd_lock);
  uint32_t next_head = (kbd_head + 1) % KBD_BUF_SIZE;
  if (next_head != kbd_tail) { /* only store if buffer isn't full */
    kbd_buf[kbd_head] = c;
    kbd_head = next_head;
  }
  spinlock_release(&kbd_lock);
}

char keyboard_getchar(void) {
  char c = 0;
  spinlock_acquire(&kbd_lock);
  if (kbd_tail != kbd_head) {
    c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
  }
  spinlock_release(&kbd_lock);
  return c;
}
