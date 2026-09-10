// keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "../cpu/exception_frame.h"

void keyboard_init(void);
void keyboard_handler(struct exception_frame* ef);
char keyboard_getchar(void);

#endif  // KEYBOARD_H
