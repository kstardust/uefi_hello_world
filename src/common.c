#include "common.h"
#include "string_utils.h"

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
cstrtowstr(char *str, CHAR16 *wstr)
{    
    UINTN len = strlen(str);
    UINTN i;

    wstr = pool_malloc(sizeof(CHAR16) * (len + 1));

    if (wstr == NULL) {
        print(L"Unable to allocate memory for wstr\r\n");
        return NULL;
    }

    for (i = 0; i < len; i++) {
        wstr[i] = str[i];
    }

    wstr[i] = '\0';

    return wstr;
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
 