// --------------------------------
// FÈUE File CoreSys Cyber::Boot
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

// All std includes are in ../main.c
#include <boot/headers.h> // Project headers
#include <core/efi.h>        // Basic UEFI types

EFI_STATUS EFIAPI cb(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    printf(L"Cyber::Boot as of 2026-08-16 11:30 Summer Stockholm time, Cyber::Boot has reached End of Life Support If you want to access Cyber::Boot click enter from here and navigate to Extra Functions to access all Cyber::Boot Features.\r\n");
    printf(L"Press any key to continue...\r\n\r\n");
    get_key();

    bmain_main(ImageHandle, SystemTable);
    return EFI_SUCCESS;
}
