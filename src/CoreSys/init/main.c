// --------------------------------
// FÈUE File CoreSys Init
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

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

void mem_write(int debug) {
    volatile uint8_t *slot = (volatile uint8_t*)KDI;
    *slot = (uint8_t)debug;

    volatile EFI_SYSTEM_TABLE **slot1 = (volatile EFI_SYSTEM_TABLE **)KSP;
    *slot1 = gST;
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
                    if (selected == 2) shutdown();
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
