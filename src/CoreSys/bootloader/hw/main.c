// --------------------------------
// FÈUE File CoreSys HW Init
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

// All std includes are in ../main.c
#include <boot/headers.h> // Project headers
#include <hw/std.h>        // Standard library for HW layer
#include <core/efi.h>        // Basic UEFI types
#include <protocols/GOP.h>
#include <protocols/ACPI.h>
#include <project/compGUID.h>
#include <stdbool.h>
#include <init/kargs.h>        // Kernel arguments structure
#include <init/headers.h>       // Kernel header structures

int kmain(kargs *Args);

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

int kernel(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, int DAUDA, int safe) {
    (void)ImageHandle;
    kargs Args = {0};

    // Fill Args with anything kmain might need
    Args.DAUDA = DAUDA;
    Args.safe = safe;

    // Optional: Fill other fields if needed
    get_memory_map(SystemTable, &Args);
    get_framebuffer(SystemTable, &Args);
    get_acpi_rsdp(SystemTable, &Args);

    static CHAR16 cmdline[256] = {0};
    Args.CmdLine = cmdline;

    Args.SystemTable = SystemTable;

    // Call kmain and return its result
    return kmain(&Args);
}

// =========================================================
// Show Menu Function
// =========================================================
void ShowIMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"CoreSys Init HW UEFI Version\r\n\r\n");
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

    ShowIMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    ShowIMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    ShowIMenu(cout, selected);
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

EFI_STATUS EFIAPI hw_imain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    imain_main(ImageHandle, SystemTable);

    return EFI_SUCCESS;
}


// =========================================================
// START OF HW_KERNEL.C
// =========================================================

int kmain(kargs *Args) {
    Args->SystemTable->ConOut->ClearScreen(Args->SystemTable->ConOut);
    Args->SystemTable->ConOut->OutputString(Args->SystemTable->ConOut, L"Hello from the HW kernel!\r\n");
    while (1) {
    }

    return Args->DAUDA;
}
