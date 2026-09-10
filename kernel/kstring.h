/* kstring.h */
#ifndef KSTRING_H
#define KSTRING_H
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int val, size_t n);
int memcmp(const void *a, const void *b, size_t n);

#endif