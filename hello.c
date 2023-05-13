#include <efi.h>
#include <efilib.h>

/*
 see: gnu-efi/lib/init.c
 * Calls to memset/memcpy may be emitted implicitly by GCC or MSVC
 * even when -ffreestanding or /NODEFAULTLIB are in effect.
 */

void *memset(void *s, int c, __SIZE_TYPE__ n)
{
    unsigned char *p = s;

    while (n--)
        *p++ = c;

    return s;
}

void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n)
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

EFI_STATUS
check_gop()
{   
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    EFI_STATUS status;
    UINTN size_of_info;

    status = BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
    
    if (EFI_ERROR(status)) {
        ST->ConOut->OutputString(ST->ConOut, L"Unable to locate protocol");        
        ST->ConOut->OutputString(ST->ConOut, itos(status & ~EFI_ERROR_MASK));
        return status;
    }    
 
    status = gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &size_of_info, &info);
    // this is needed to get the current video mode
    if (status == EFI_NOT_STARTED) 
        status = gop->SetMode(gop, 0);    

    if (EFI_ERROR(status)) {
        ST->ConOut->OutputString(ST->ConOut, L"Unable to get native mode");
        return status;
    }

    info = gop->Mode->Info;    

    ST->ConOut->OutputString(ST->ConOut, L"width: ");
    ST->ConOut->OutputString(ST->ConOut, itos(info->HorizontalResolution));
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    ST->ConOut->OutputString(ST->ConOut, L"height: ");
    ST->ConOut->OutputString(ST->ConOut, itos(info->VerticalResolution));
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    ST->ConOut->OutputString(ST->ConOut, L"FrameBufferSize: ");
    ST->ConOut->OutputString(ST->ConOut, itos(gop->Mode->FrameBufferSize));
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    ST->ConOut->OutputString(ST->ConOut, L"FrameBufferBase: ");
    ST->ConOut->OutputString(ST->ConOut, itos(gop->Mode->FrameBufferBase));
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    ST->ConOut->OutputString(ST->ConOut, L"PixelsPerScanLine: ");
    ST->ConOut->OutputString(ST->ConOut, itos(info->PixelsPerScanLine));
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");        

    
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixels[30 * 3] = {
        {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, 
        {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, 

        {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, 
        {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, 

        {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, 
        {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, {00, 0xff, 00, 0xff}, 

        {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, 
        {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, 

        {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, 
        {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, 

        {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, 
        {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, {0xff, 0x00, 00, 0xff}, 

        {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, 
        {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, 

        {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, 
        {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, 
        
        {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, 
        {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, {00, 00, 0xff, 0xff}, 
                
    };

    status = gop->Blt(gop, 
            pixels,
            EfiBltBufferToVideo,
            0,
            0,
            0,
            0,
            10,
            9,
            sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * 10);
    
    if (EFI_ERROR(status)) {
        ST->ConOut->OutputString(ST->ConOut, L"Unable to blt");
        return status;
    }

    return EFI_SUCCESS;
}

void
ClearScreen()
{
    ST->ConOut->ClearScreen(ST->ConOut);
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    EFI_INPUT_KEY key;
 
    /* Store the system table for future use in other functions */
    ST = SystemTable;
    BS = ST->BootServices;

    ClearScreen();
    /* Say hi */
    status = ST->ConOut->OutputString(ST->ConOut, L"Hello World\r\n"); // EFI Applications use Unicode and CRLF, a la Windows
    if (EFI_ERROR(status))
        return status;
    
    status = check_gop();
    /* Now wait for a keystroke before continuing, otherwise your
       message will flash off the screen before you see it.
 
       First, we need to empty the console input buffer to flush
       out any keystrokes entered before this point */
    status = ST->ConIn->Reset(ST->ConIn, FALSE);
    if (EFI_ERROR(status))
        return status;
 
    /* Now wait until a key becomes available.  This is a simple
       polling implementation.  You could try and use the WaitForKey
       event instead if you like */
    while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key)) == EFI_NOT_READY) ;
 
    return status;
}
