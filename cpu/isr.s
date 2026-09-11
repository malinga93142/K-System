.intel_syntax noprefix

.extern exception_handler

.macro ISR_NOERR num
.globl isr\num
.type isr\num, @function
isr\num:
  push 0
  push \num
  jmp isr_common_stub
.endm

.macro ISR_ERR num
.globl isr\num
.type isr\num, @function
isr\num:
  push \num
  jmp isr_common_stub
.endm

isr_common_stub:
  pusha                  
  mov ax, ds              
  push eax

  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  push esp                 
  call exception_handler
  add esp, 4

  pop eax
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  popa
  add esp, 8
  iret

ISR_NOERR 48
ISR_ERR 8
ISR_ERR 14
ISR_NOERR 64
ISR_NOERR 65

# IRQs
ISR_NOERR 32
ISR_NOERR 33


ISR_NOERR 241
