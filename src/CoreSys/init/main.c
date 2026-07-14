// -----------------------------------------
// FÈUE File CoreSys Init
// Contains code of the project
// CyberBoot by FÈUE
// INIT / OS
// Namespace io.feue.coresys.init.imain
// -----------------------------------------

#include <core/efi.h>
#include <core/efi_lib.h>
#include <arch/x86_64.h>
#include <char/string_funcs.h>
#include <core/stdio_efi.h>
#include <project/codes.h>
#include <project/power.h>
#include <protocols/SFS.h>
#include <protocols/GOP.h>
#include <protocols/ACPI.h>
#include <project/compGUID.h>
#include <stdbool.h>
#include <kernel/mem.h>
#include <init/headers.h>
#include <API/CoreSys.h> // CoreSys API

void init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    image = ImageHandle;
    st    = SystemTable;
    bs    = SystemTable->BootServices;
    rs    = SystemTable->RuntimeServices;

    gST = SystemTable;
    gBS = SystemTable->BootServices;
    gRS = SystemTable->RuntimeServices;

    cout = SystemTable->ConOut;
    cin  = SystemTable->ConIn;
}

BOOT_INFO boot_info;

void get_memory_mapl(EFI_SYSTEM_TABLE *SystemTable)
{
    UINTN memory_map_size = 0;
    UINTN map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;

    EFI_STATUS status;


    /*
        First call gets required size
    */
    status = SystemTable->BootServices->GetMemoryMap(
        &memory_map_size,
        NULL,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );


    memory_map_size += descriptor_size * 8;


    EFI_MEMORY_DESCRIPTOR *memory_map;


    status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        memory_map_size,
        (VOID**)&memory_map
    );


    if (EFI_ERROR(status))
    {
        printf(L"Failed allocating memory map\n");
        return;
    }


    status = SystemTable->BootServices->GetMemoryMap(
        &memory_map_size,
        memory_map,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );


    if (EFI_ERROR(status))
    {
        printf(L"GetMemoryMap failed\n");
        return;
    }



    boot_info.memory_entries = 0;
    boot_info.installed_ram = 0;
    boot_info.reserved_ram = 0;



    for(UINTN i = 0; i < memory_map_size / descriptor_size; i++)
    {

        EFI_MEMORY_DESCRIPTOR *desc =
            (EFI_MEMORY_DESCRIPTOR*)
            ((UINT8*)memory_map + i * descriptor_size);



        if(boot_info.memory_entries < MAX_MEMORY_ENTRIES)
        {
            boot_info.memory_map[
                boot_info.memory_entries
            ].base =
                desc->PhysicalStart;


            boot_info.memory_map[
                boot_info.memory_entries
            ].length =
                desc->NumberOfPages * 4096;


            boot_info.memory_map[
                boot_info.memory_entries
            ].type =
                desc->Type;


            boot_info.memory_entries++;
        }



        switch(desc->Type)
        {

            case EfiConventionalMemory:

            case EfiLoaderCode:

            case EfiLoaderData:

            case EfiBootServicesCode:

            case EfiBootServicesData:

                boot_info.installed_ram +=
                    desc->NumberOfPages * 4096;

                break;



            case EfiReservedMemoryType:

            case EfiUnusableMemory:

            case EfiACPIReclaimMemory:

            case EfiACPIMemoryNVS:

                boot_info.reserved_ram +=
                    desc->NumberOfPages * 4096;

                break;



            case EfiMemoryMappedIO:

            case EfiMemoryMappedIOPortSpace:

                break;

        }

    }



    /*
        x86_64 usually supports 40-52 bits.
        CPUID should replace this later.
    */
    boot_info.physical_address_width = 40;

    boot_info.max_supported_memory =
        1ULL << boot_info.physical_address_width;



    SystemTable->BootServices->FreePool(memory_map);
}

// -------------------------------
// Load EFI image
// -------------------------------
EFI_STATUS load_efi(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    CHAR16 *FilePath,
    EFI_HANDLE *OutImage
)
{
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
    EFI_FILE_PROTOCOL *Root = NULL, *File = NULL;
    EFI_STATUS Status;
    void *Buffer = NULL;
    EFI_FILE_INFO *Info = NULL;

    UINTN FileSize;
    UINTN InfoSize = sizeof(EFI_FILE_INFO) + 200;

    Status = SystemTable->BootServices->LocateProtocol(
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        (void**)&Fs
    );
    if (EFI_ERROR(Status)) return Status;

    Status = Fs->OpenVolume(Fs, &Root);
    if (EFI_ERROR(Status)) return Status;

    Status = Root->Open(Root, &File, FilePath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) return Status;

    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        InfoSize,
        (void**)&Info
    );
    if (EFI_ERROR(Status)) goto cleanup_file;

    Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, Info);
    if (EFI_ERROR(Status)) goto cleanup_info;

    FileSize = (UINTN)Info->FileSize;

    if (FileSize <= 2)
    {
        Status = EFI_INVALID_PARAMETER;
        goto cleanup_info;
    }

    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        FileSize,
        &Buffer
    );
    if (EFI_ERROR(Status)) goto cleanup_info;

    Status = File->Read(File, &FileSize, Buffer);
    if (EFI_ERROR(Status)) goto cleanup_buffer;

    UINT8 *Raw = (UINT8 *)Buffer;
    UINT8 *Adjusted = Raw + 2;
    UINTN AdjustedSize = FileSize - 2;

    EFI_HANDLE LoadedImage = NULL;

    Status = SystemTable->BootServices->LoadImage(
        FALSE,
        ImageHandle,
        NULL,
        Adjusted,
        AdjustedSize,
        &LoadedImage
    );

    if (EFI_ERROR(Status)) goto cleanup_buffer;

    *OutImage = LoadedImage;

cleanup_buffer:
    if (Buffer) SystemTable->BootServices->FreePool(Buffer);

cleanup_info:
    if (Info) SystemTable->BootServices->FreePool(Info);

cleanup_file:
    if (File) File->Close(File);

    return Status;
}

// -------------------------------
// Jump to kernel (NO ARGS)
// -------------------------------
EFI_STATUS kjump(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE LoadedImage)
{
    return SystemTable->BootServices->StartImage(
        LoadedImage,
        NULL,
        NULL
    );
}

// -------------------------------
// Unload EFI image
// -------------------------------
EFI_STATUS unload_efi(EFI_HANDLE LoadedImage, EFI_SYSTEM_TABLE *SystemTable)
{
    return SystemTable->BootServices->UnloadImage(LoadedImage);
}

// -------------------------------
// Boot helper
// -------------------------------
EFI_STATUS kboot(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE* SystemTable,
    CHAR16* name
)
{
    EFI_HANDLE LoadedImage = NULL;

    EFI_STATUS Status = load_efi(ImageHandle, SystemTable, name, &LoadedImage);
    if (EFI_ERROR(Status))
        goto cleanup;

    Status = kjump(SystemTable, LoadedImage);

cleanup:
    printf(L"Returned from kernel (Press any key to continue...)\r\n");
    get_key();

    if (LoadedImage)
        unload_efi(LoadedImage, SystemTable);

    return Status;
}

void get_acpi_info(EFI_SYSTEM_TABLE *SystemTable)
{
    boot_info.rsdp = 0;
    boot_info.rsdt = 0;
    boot_info.xsdt = 0;
    boot_info.acpi_available = 0;


    for(UINTN i = 0;
        i < SystemTable->NumberOfTableEntries;
        i++)
    {

        EFI_CONFIGURATION_TABLE *table =
            &SystemTable->ConfigurationTable[i];


        if(memcmp(
            &table->VendorGuid,
            &gEfiAcpi20TableGuid,
            sizeof(EFI_GUID)) == 0)
        {

            ACPI_RSDP *rsdp =
                (ACPI_RSDP*)table->VendorTable;


            boot_info.rsdp =
                (uint64_t)rsdp;


            boot_info.acpi_revision =
                rsdp->Revision;


            boot_info.acpi_available = 1;


            if(rsdp->Revision >= 2)
            {
                boot_info.xsdt =
                    rsdp->XsdtAddress;
            }
            else
            {
                boot_info.rsdt =
                    rsdp->RsdtAddress;
            }


            break;
        }


        if(memcmp(
            &table->VendorGuid,
            &gEfiAcpi10TableGuid,
            sizeof(EFI_GUID)) == 0)
        {

            ACPI_RSDP *rsdp =
                (ACPI_RSDP*)table->VendorTable;


            boot_info.rsdp =
                (uint64_t)rsdp;


            boot_info.rsdt =
                rsdp->RsdtAddress;


            boot_info.acpi_revision =
                rsdp->Revision;


            boot_info.acpi_available = 1;

            break;
        }
    }
}

void get_framebuffer() {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    EFI_STATUS Status;

    Status = gST->BootServices->LocateProtocol(&gopGuid, NULL, (VOID**)&gop);

    if(EFI_ERROR(Status))
    {
        printf(L"GOP not found\n");
        return;
    }

    boot_info.framebuffer = 0;
    boot_info.framebuffer_size = 0;
    boot_info.width = 0;
    boot_info.height = 0;
    boot_info.pitch = 0;
    boot_info.format = 0;

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info =
        gop->Mode->Info;

    boot_info.framebuffer =
        gop->Mode->FrameBufferBase;

    boot_info.framebuffer_size =
        gop->Mode->FrameBufferSize;

    boot_info.width =
        info->HorizontalResolution;

    boot_info.height =
        info->VerticalResolution;

    boot_info.pitch =
        info->PixelsPerScanLine;

    boot_info.format =
        info->PixelFormat;
}

void mem_write(int debug) {
    get_memory_mapl(gST);
    get_acpi_info(gST);
    get_framebuffer();

    volatile uint8_t *slot = (volatile uint8_t*)KDI;
    *slot = (uint8_t)debug; 

    volatile BOOT_INFO *slot1 = (volatile BOOT_INFO*)KSP;
    *slot1 = boot_info;
    
}

// -------------------------------
// Kernel entry UI
// -------------------------------
void kernel(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, int debug)
{
    clear_screen();

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Jump to Kernel\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press ESC to return\r\n");

    EFI_INPUT_KEY key;
    UINTN index;

    SystemTable->BootServices->WaitForEvent(
        1,
        &SystemTable->ConIn->WaitForKey,
        &index
    );

    SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

    if (key.ScanCode == SCANCODE_ESC)
    {
        imain_main(ImageHandle, SystemTable);
        return;
    }

    mem_write(debug);

    EFI_STATUS Status = kboot(
        ImageHandle,
        SystemTable,
        L"\\EFI\\FEUE\\KERNEL.RE"
    );

    printf(L"Kernel returned: 0x%llx\r\n", (unsigned long long)Status);
}

// -------------------------------
// Menu
// -------------------------------
void ShowMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected)
{
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, L"CoreSys Init UEFI Version\r\n\r\n");
    cout->OutputString(cout, L"W/S navigation, ENTER select\r\n\r\n");

    CHAR16* options[] =
    {
        L"CoreSys Kernel (Normal)",
        L"CoreSys Kernel (Debug)",
        L"CoreSys Kernel (Silent (not recommened))",
        L"Shutdown"
    };

    for (UINTN i = 0; i < 4; i++)
    {
        if (i == selected)
            cout->OutputString(cout, L"> ");
        else
            cout->OutputString(cout, L"  ");

        cout->OutputString(cout, options[i]);
        cout->OutputString(cout, L"\r\n");
    }
}

// -------------------------------
// Main menu loop
// -------------------------------
void imain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    UINTN selected = 0;
    EFI_INPUT_KEY key;

    ShowMenu(cout, selected);

    while (1)
    {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS)
        {
            switch (key.UnicodeChar)
            {
                case L'w':
                case L'W':
                    selected = (selected == 0) ? 3 : selected - 1;
                    ShowMenu(cout, selected);
                    break;

                case L's':
                case L'S':
                    selected = (selected + 1) % 4;
                    ShowMenu(cout, selected);
                    break;

                case L'\r':
                    if (selected == 0) kernel(ImageHandle, SystemTable, 0);
                    if (selected == 1) kernel(ImageHandle, SystemTable, 1);
                    if (selected == 2) kernel(ImageHandle, SystemTable, 2);
                    if (selected == 3) shutdown();
                    break;
            }
        }
    }
}

// -------------------------------
// Entry
// -------------------------------
EFI_STATUS EFIAPI imain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    init(ImageHandle, SystemTable);
    clear_screen();

    cs_logf(CS_LOG_INFO, u"CoreSys Init has been boot successfully");
    cs_logf(CS_LOG_INFO, u"Press any key to continue...");

    get_key();

    imain_main(ImageHandle, SystemTable);

    return EFI_SUCCESS;
}
