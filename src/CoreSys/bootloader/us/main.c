// --------------------------------
// FÈUE File CoreSys UEFI Shell
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

// All std includes are in ../main.c
#include <boot/headers.h> // Project headers
#include <core/efi.h>        // Basic UEFI types

// Main UEFI initialization
void us_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    if (!SystemTable) return;

    clear_screen();
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Jump to UEFI shell\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press the ESC key to exit...\r\n");
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
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"ESC pressed. Returning...\r\n");
        bmain_main(ImageHandle, SystemTable);
        return;
    }

    // Continue normally
    EFI_STATUS Status = boot(ImageHandle, SystemTable, L"\\EFI\\UEFI\\SHELLX64.EFI");

    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Boot failed\r\n");
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press any key to continue...\r\n");

        // wait again
        SystemTable->BootServices->WaitForEvent(
            1,
            &SystemTable->ConIn->WaitForKey,
            &index
        );
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

        bmain_main(ImageHandle, SystemTable);
    }

    // Cleanup (safe but currently unused)
    EFI_FILE_PROTOCOL *File = NULL;
    void *Buffer = NULL;
    EFI_FILE_INFO *Info = NULL;

    if (Buffer) SystemTable->BootServices->FreePool(Buffer);
    if (Info) SystemTable->BootServices->FreePool(Info);
    if (File) File->Close(File);
}
