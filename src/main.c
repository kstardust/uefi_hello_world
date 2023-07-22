#include <efi.h>
#include <efilib.h>
#include "common.h"
#include "string_utils.h"

void putchar(int c)
{
    static CHAR16 buf[2];
    buf[0] = c;
    ST->ConOut->OutputString(ST->ConOut, buf); 
}

void* malloc(size_t size)
{
    return pool_malloc(size);
}

const char *
efi_memory_attr(EFI_MEMORY_DESCRIPTOR *p)
{
    if (p->Attribute & EFI_MEMORY_UC)
        return "UC ";
    if (p->Attribute & EFI_MEMORY_WC)
        return "WC ";
    if (p->Attribute & EFI_MEMORY_WT)
        return "WT ";
    if (p->Attribute & EFI_MEMORY_WB)
        return "WB ";
    if (p->Attribute & EFI_MEMORY_UCE)
        return "UCE ";
    if (p->Attribute & EFI_MEMORY_WP)
        return "WP ";
    if (p->Attribute & EFI_MEMORY_RP)
        return "RP ";
    if (p->Attribute & EFI_MEMORY_XP)
        return "XP ";
    return "Unknown";
}

const char *
efi_memory_type(EFI_MEMORY_TYPE type)
{
	const char *types[] = {
	    "Reserved",
	    "LoaderCode",
	    "LoaderData",
	    "BootServicesCode",
	    "BootServicesData",
	    "RuntimeServicesCode",
	    "RuntimeServicesData",
	    "ConventionalMemory",
	    "UnusableMemory",
	    "ACPIReclaimMemory",
	    "ACPIMemoryNVS",
	    "MemoryMappedIO",
	    "MemoryMappedIOPortSpace",
	    "PalCode",
	    "PersistentMemory"
	};

	switch (type) {
	case EfiReservedMemoryType:
	case EfiLoaderCode:
	case EfiLoaderData:
	case EfiBootServicesCode:
	case EfiBootServicesData:
	case EfiRuntimeServicesCode:
	case EfiRuntimeServicesData:
	case EfiConventionalMemory:
	case EfiUnusableMemory:
	case EfiACPIReclaimMemory:
	case EfiACPIMemoryNVS:
	case EfiMemoryMappedIO:
	case EfiMemoryMappedIOPortSpace:
	case EfiPalCode:
		return (types[type]);
	default:
		return ("Unknown");
	}
}

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
        printf("Unable to locate protocol");                
        return status;
    }    
 
    status = gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &size_of_info, &info);
    // this is needed to get the current video mode
    if (status == EFI_NOT_STARTED) 
        status = gop->SetMode(gop, 0);    

    if (EFI_ERROR(status)) {
        printf("Unable to get native mode");
        return status;
    }

    info = gop->Mode->Info;    
      
    height = info->VerticalResolution;
    width = info->HorizontalResolution;
    pixels_per_scanline = info->PixelsPerScanLine;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pixels = pool_malloc(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * pixels_per_scanline * height);

    if (pixels == NULL) {
        printf("Unable to allocate memory for pixels");
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
    
    

    if (EFI_ERROR(status)) {
        printf("Unable to blt");
        return status;
    }

    printf("width: %d\r\n", width);
    printf("height: %d\r\n", height);
    printf("FrameBufferSize: 0x%x\r\n", gop->Mode->FrameBufferSize);
    printf("FrameBufferBase: 0x%x\r\n", gop->Mode->FrameBufferBase);
    printf("PixelsPerScanLine: %d\r\n", info->PixelsPerScanLine);

    pool_free(pixels);
    return EFI_SUCCESS;
}

void
ClearScreen()
{
    ST->ConOut->ClearScreen(ST->ConOut);
}

void
get_memory_map()
{
    UINTN map_size;
    EFI_MEMORY_DESCRIPTOR *mem_map = NULL;
    UINTN map_key;
    UINTN desc_size;
    UINT32 desc_ver;
    EFI_STATUS status;
    map_size = 0;
    
    for (;;) {
        status = BS->GetMemoryMap(&map_size, mem_map, &map_key, &desc_size, &desc_ver);
        if (!EFI_ERROR(status))
            break;
                
        if (status != EFI_BUFFER_TOO_SMALL) {
            printf("Unable to get memory map %d\r\n", status);
            return;
        }

        if (mem_map != NULL) {
            pool_free(mem_map);
        }
        mem_map = pool_malloc(map_size);
    }   
    printf("mem_map: 0x%x\r\n", mem_map);
    printf("Memory Map\r\n");
    printf("type \t physical_start \t virtual_start \t number_of_pages \t attribute\r\n");
    for (int i = 0; i < map_size; i+=desc_size) {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)mem_map + i);
        printf("%s \t 0x%x \t 0x%x \t %d \t %s\r\n", 
        efi_memory_type(desc->Type), desc->PhysicalStart, desc->VirtualStart, desc->NumberOfPages, 
        efi_memory_attr(desc));        
    }    
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
    printf("Hello, World!\r\n");
    if (EFI_ERROR(status))
        return status;
    
    status = check_gop();
    get_memory_map();
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
