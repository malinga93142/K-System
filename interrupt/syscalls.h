#ifndef SYSCALLS_H
#define SYSCALLS_H
#include "../cpu/exception_frame.h"
void syscall_handler(struct exception_frame *);
#endif