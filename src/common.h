#ifndef _MEFI_COMMON
#define _MEFI_COMMON

#include <efi.h>
#include <efilib.h>

void* memset(void *s, int c, __SIZE_TYPE__ n);
void* memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
CHAR16* itos(uint64_t n);
void pool_free(void *p);
void* pool_malloc(UINTN size);
void print(CHAR16* msg);

#endif