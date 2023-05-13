#include <efi.h>
#include <efilib.h>
#include "common.h"


EFI_STATUS
check_gop()
{   
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    EFI_STATUS status;
    UINTN size_of_info, width, height, pixels_per_scanline;

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
      
    height = info->VerticalResolution;
    width = info->HorizontalResolution;
    pixels_per_scanline = info->PixelsPerScanLine;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pixels = pool_malloc(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * pixels_per_scanline * height);

    if (pixels == NULL) {
        ST->ConOut->OutputString(ST->ConOut, L"Unable to allocate memory for pixels");
        return EFI_OUT_OF_RESOURCES;
    }    

    for (int i = 0; i < pixels_per_scanline * height; i++) {
        pixels[i].Blue = 0;
        pixels[i].Green = 0xff;
        pixels[i].Red = 0;
        pixels[i].Reserved = 0xff;
    }

    status = gop->Blt(gop, 
            pixels,
            EfiBltBufferToVideo,
            0,
            0,
            0,
            0,
            pixels_per_scanline,
            height,
            sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * pixels_per_scanline);
    
    
    ST->ConOut->OutputString(ST->ConOut, itos(~EFI_ERROR_MASK & status));    

    if (EFI_ERROR(status)) {
        ST->ConOut->OutputString(ST->ConOut, L"Unable to blt");
        return status;
    }

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

    pool_free(pixels);
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
