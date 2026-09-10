#ifndef EXCEPTION_FRAME_H
#define EXCEPTION_FRAME_H
#include <stdint.h>
struct exception_frame
{
  uint32_t ds;                                           /* pushed last, so lowest addr */
  uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax; /* pusha order */
  uint32_t int_no;
  uint32_t error_code;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
};
#endif