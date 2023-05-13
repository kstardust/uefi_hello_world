#include "common.h"

/* see: gnu-efi/lib/init.c
 * Calls to memset/memcpy may be emitted implicitly by GCC or MSVC
 * even when -ffreestanding or /NODEFAULTLIB are in effect.
 */
void*
memset(void *s, int c, __SIZE_TYPE__ n)
{
    unsigned char *p = s;

    while (n--)
        *p++ = c;

    return s;
}

void*
memcpy(void *dest, const void *src, __SIZE_TYPE__ n)
{
    const unsigned char *q = src;
    unsigned char *p = dest;

    while (n--)
        *p++ = *q++;

    return dest;
}

CHAR16*
itos(uint64_t n)
{
    static CHAR16 buf[21];
    CHAR16 *p = buf + 20;
    *p = '\0';
    do {
        *--p = (n % 10) + '0';
        n /= 10;
    } while (n != 0);
    return p;
}

void 
print(CHAR16* msg)
{
    ST->ConOut->OutputString(ST->ConOut, msg);
}

void*
pool_malloc(UINTN size)
{
    void *p;
    EFI_STATUS status;

    status = BS->AllocatePool(EfiLoaderData, size, (void**)&p);

    if (status == EFI_OUT_OF_RESOURCES) {        
        print(L"out of resources\r\n");
        return NULL;
    }

    if (EFI_ERROR(status)) {
        print(L"error allocating pool memory\r\n");
        return NULL;
    }

    return p;
}

void
pool_free(void *p)
{
    EFI_STATUS status;
    status = BS->FreePool(p);

    if (EFI_ERROR(status)) {
        print(L"error freeing pool memory\r\n");
    }
}