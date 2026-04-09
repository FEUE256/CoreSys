// --------------------------------
// FÈUE File CoreSys HW Init
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

// All std includes are in ../main.c
#include <boot/headers.h> // Project headers
#include <core/efi.h>        // Basic UEFI types

EFI_STATUS EFIAPI cb(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    printf(L"Download Cyber::Boot at CoreSys website!\r\n");
    printf(L"Press any key to continue...\r\n\r\n");
    get_key();

    bmain_main(ImageHandle, SystemTable);
    return EFI_SUCCESS;
}
