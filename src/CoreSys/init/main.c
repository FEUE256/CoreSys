// --------------------------------
// FÈUE File CoreSys Init
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

#include <core/efi.h>         // Basic UEFI types
#include <core/efi_lib.h>     // Simple EFI lib
#include <arch/x86_64.h>      // x86_64 definitions
#include <char/string_funcs.h> // String conversion functions
#include <core/stdio_efi.h>   // Custom printf for UEFI
#include <project/codes.h>    // Project codes/macros
#include <project/power.h>    // Power functions
#include <protocols/SFS.h>
#include <protocols/GOP.h>
#include <protocols/ACPI.h>
#include <project/compGUID.h>
#include <stdbool.h>
#include <init/kargs.h>        // Kernel arguments structure
#include <init/headers.h>       // Kernel header structures

void init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Store local handles
    image = ImageHandle;
    st    = SystemTable;
    bs    = SystemTable->BootServices;
    rs    = SystemTable->RuntimeServices;

    // Set global pointers
    gST = SystemTable;
    gBS = SystemTable->BootServices;
    gRS = SystemTable->RuntimeServices;

    // Set console protocols
    cout = SystemTable->ConOut;
    cin  = SystemTable->ConIn;
}


// Load an EFI image into memory
EFI_STATUS load_efi(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, CHAR16 *FilePath, EFI_HANDLE *OutImage) {
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
    EFI_FILE_PROTOCOL *Root = NULL, *File = NULL;
    EFI_STATUS Status;
    void *Buffer = NULL;
    EFI_FILE_INFO *Info = NULL;
    UINTN FileSize;
    UINTN InfoSize = sizeof(EFI_FILE_INFO) + 200;

    // Locate filesystem
    Status = SystemTable->BootServices->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (void**)&Fs);
    if (EFI_ERROR(Status)) return Status;

    Status = Fs->OpenVolume(Fs, &Root);
    if (EFI_ERROR(Status)) return Status;

    // Open EFI file
    Status = Root->Open(Root, &File, FilePath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) return Status;

    // Allocate memory for file info
    Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, InfoSize, (void**)&Info);
    if (EFI_ERROR(Status)) goto cleanup_file;

    Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, Info);
    if (EFI_ERROR(Status)) goto cleanup_info;

    FileSize = (UINTN)Info->FileSize;

    // Allocate buffer for file
    Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, FileSize, &Buffer);
    if (EFI_ERROR(Status)) goto cleanup_info;

    // Read file into buffer
    Status = File->Read(File, &FileSize, Buffer);
    if (EFI_ERROR(Status)) goto cleanup_buffer;

    // Load image into memory
    EFI_HANDLE LoadedImage = NULL;
    Status = SystemTable->BootServices->LoadImage(FALSE, ImageHandle, NULL, Buffer, FileSize, &LoadedImage);
    if (EFI_ERROR(Status)) goto cleanup_buffer;

    *OutImage = LoadedImage;
    Status = EFI_SUCCESS;
    goto cleanup_info;

    cleanup_buffer:
        if (Buffer) SystemTable->BootServices->FreePool(Buffer);
    cleanup_info:
        if (Info) SystemTable->BootServices->FreePool(Info);
    cleanup_file:
        if (File) File->Close(File);

    return Status;
}

EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *SystemTable, kargs *Args) {
    if (!SystemTable || !Args) return EFI_INVALID_PARAMETER;

    EFI_STATUS Status;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MemoryMapSize = 0;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    // First call to get the required buffer size
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;

    // Allocate pool for memory map
    Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, MemoryMapSize, (void**)&MemoryMap);
    if (EFI_ERROR(Status)) return Status;

    // Actually get the memory map
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        SystemTable->BootServices->FreePool(MemoryMap);
        return Status;
    }

    // Save info into kargs
    Args->MemoryMap = MemoryMap;
    Args->MemoryMapSize = MemoryMapSize;
    Args->DescriptorSize = DescriptorSize;
    Args->DescriptorVersion = DescriptorVersion;

    return EFI_SUCCESS;
}

EFI_STATUS get_framebuffer(EFI_SYSTEM_TABLE *SystemTable, kargs *Args) {
    if (!SystemTable || !Args) return EFI_INVALID_PARAMETER;

    EFI_STATUS Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP;

    // Locate the Graphics Output Protocol
    Status = SystemTable->BootServices->LocateProtocol(
        (EFI_GUID*)&gEfiGraphicsOutputProtocolGuid, // cast away const
        NULL,
        (void**)&GOP
    );
    if (EFI_ERROR(Status)) return Status;

    // Save framebuffer info into kargs
    Args->FramebufferBase   = (VOID*)GOP->Mode->FrameBufferBase;
    Args->FramebufferWidth  = GOP->Mode->Info->HorizontalResolution;
    Args->FramebufferHeight = GOP->Mode->Info->VerticalResolution;
    Args->FramebufferBPP    = GOP->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor ||
                              GOP->Mode->Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor ? 32 : 0; // assume 32-bit common
    Args->FramebufferPitch  = GOP->Mode->Info->PixelsPerScanLine * (Args->FramebufferBPP / 8);

    return EFI_SUCCESS;
}

EFI_STATUS get_acpi_rsdp(EFI_SYSTEM_TABLE *SystemTable, kargs *Args) {
    if (!SystemTable || !Args) return EFI_INVALID_PARAMETER;

    EFI_CONFIGURATION_TABLE *ConfigTable = SystemTable->ConfigurationTable;
    UINTN NumTables = SystemTable->NumberOfTableEntries;

    for (UINTN i = 0; i < NumTables; i++) {
        // Check for ACPI 2.0 RSDP GUID first
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiAcpi20TableGuid)) {
            Args->Rsdp = ConfigTable[i].VendorTable;
            return EFI_SUCCESS;
        }
        // Fallback to ACPI 1.0 if 2.0 not found
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiAcpi10TableGuid)) {
            Args->Rsdp = ConfigTable[i].VendorTable;
            return EFI_SUCCESS;
        }
    }

    // RSDP not found
    Args->Rsdp = NULL;
    return EFI_NOT_FOUND;
}

EFI_STATUS kjump(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE LoadedImage, int DAUDA, int safe) {
    if (!LoadedImage || !SystemTable) return EFI_INVALID_PARAMETER;

    kargs Args = {0};
    EFI_STATUS Status;

    // Memory map
    Status = get_memory_map(SystemTable, &Args);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to get memory map\r\n");
        return Status;
    }

    // Framebuffer
    Status = get_framebuffer(SystemTable, &Args);
    if (EFI_ERROR(Status)) return Status;

    // ACPI RSDP
    Status = get_acpi_rsdp(SystemTable, &Args);
    if (EFI_ERROR(Status)) {
        printf(L"ACPI RSDP not found!\n");
    } else {
        printf(L"ACPI RSDP located at: %u\n", (UINTN)Args.Rsdp);
    }

    Args.DAUDA = DAUDA;
    Args.safe = safe;

    static CHAR16 cmdline[256] = {0};
    Args.CmdLine = cmdline;

    UINTN MapSize = Args.MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR* MemoryMap = Args.MemoryMap;
    UINTN MapKey;
    UINTN DescriptorSize = Args.DescriptorSize;
    UINT32 DescriptorVersion = Args.DescriptorVersion;

    // 1. First, query the required buffer size
    Status = SystemTable->BootServices->GetMemoryMap(&MapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    Args.SystemTable = SystemTable;

    // No exit boot servies

    // Kernel starts here. We jump to the loaded image, passing the kargs pointer in RDI (first argument in System V AMD64 ABI)
    return SystemTable->BootServices->StartImage(LoadedImage, NULL, (VOID*)&Args);
    SystemTable->BootServices->FreePool(MemoryMap); // Free the pool allocated in get_memory_map
}

// Unload an EFI image
EFI_STATUS unload_efi(EFI_HANDLE LoadedImage, EFI_SYSTEM_TABLE *SystemTable) {
    if (!LoadedImage || !SystemTable) return EFI_INVALID_PARAMETER;
    return SystemTable->BootServices->UnloadImage(LoadedImage);
}

// Boot helper: load, start, then return
EFI_STATUS kboot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* name, int DAUDA, int safe) {
    if (!SystemTable || !name) return EFI_INVALID_PARAMETER;

    EFI_STATUS Status;
    EFI_HANDLE LoadedImage = NULL;

    Status = load_efi(ImageHandle, SystemTable, name, &LoadedImage);
    if (EFI_ERROR(Status)) {
        goto cleanup;
    }

    Status = kjump(SystemTable, LoadedImage, DAUDA, safe);
    if (EFI_ERROR(Status)) {
        goto cleanup;
    }

    cleanup:
        printf(L"Return. Press any key to continue...\r\n");
        get_key();
        if (LoadedImage) {
            unload_efi(LoadedImage, SystemTable);
        }

    return Status;
}

void kernel(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, int DAUDA, int safe) {
    clear_screen();
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Jump to Kernel\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press the ESC key to go back...\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press any other key to continue...\r\n");

    // Wait for key
    EFI_INPUT_KEY key;
    UINTN index;

    SystemTable->BootServices->WaitForEvent(
        1,
        &SystemTable->ConIn->WaitForKey,
        &index
    );

    SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

    // ESC handling (ScanCode, not Unicode)
    if (key.ScanCode == SCANCODE_ESC) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"ESC pressed. Shutdown...\r\n");
        imain_main(ImageHandle, SystemTable); // Show menu again
        return;
    }

    // Continue normally
    EFI_STATUS Status = kboot(ImageHandle, SystemTable, L"\\EFI\\FEUE\\KERNEL.EFI", DAUDA, safe);

    printf(L"Returned from kernel with status: 0x%u\r\n", Status);
    printf(L"You need to shutdown with the button...\r\n");

}

// =========================================================
// Show Menu Function
// =========================================================
void ShowMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"CoreSys Init UEFI Version\r\n\r\n");
    cout->OutputString(cout, (CHAR16 *)L"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"CoreSys (Normal)",
        L"CoreSys (DAUDA (Debug and Allow Unsiged Drivers and Applications))",
        L"CoreSys (Safe Mode)",
        L"Shutdown"
    };
    const UINTN optionCount = sizeof(options) / sizeof(options[0]);

    for (UINTN i = 0; i < optionCount; i++) {
        if (i == selected) {
            cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));
            cout->OutputString(cout, (CHAR16 *)L"> ");
            cout->OutputString(cout, (CHAR16 *)options[i]);
            cout->OutputString(cout, (CHAR16 *)L"\r\n");
            cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
        } else {
            cout->OutputString(cout, (CHAR16 *)L"  ");
            cout->OutputString(cout, (CHAR16 *)options[i]);
            cout->OutputString(cout, (CHAR16 *)L"\r\n");
        }
    }
}

void imain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    UINTN selected = 0;
    EFI_INPUT_KEY key;
    const UINTN optionCount = 4;

    ShowMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    ShowMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    ShowMenu(cout, selected);
                    break;
                case L'\r':  // Enter
                switch (selected) {
                    case 0: kernel(ImageHandle, SystemTable, 0, 0); break;
                    case 1: kernel(ImageHandle, SystemTable, 1, 0); break;
                    case 2: kernel(ImageHandle, SystemTable, 0, 1); break;
                    case 3: shutdown(); break;
                }
                break;
            }
        }
    }
}

EFI_STATUS EFIAPI imain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    imain_main(ImageHandle, SystemTable);

    return EFI_SUCCESS;
}
