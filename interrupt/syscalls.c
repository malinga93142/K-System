#include "syscalls.h"

#include "../drivers/uart.h"
#include "../drivers/vga.h"

void syscall_handler(struct exception_frame* ef) {
  switch (ef->int_no) {
    case 48:
      break;
    case 64:
      break;
    case 65: {
      const char* buf = (const char*)ef->edx;
      uint32_t len = ef->ecx;
      for (uint32_t i = 0; i < len; i++) {
        vga_putc(buf[i]);
      }
      break;
    }
    case 0x80: {
			switch(ef->eax) {
				case 1:
				case 2:
				case 3:
			}
    }
  }
}
