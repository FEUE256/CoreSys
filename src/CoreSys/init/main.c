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

EFI_STATUS EFIAPI imain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    printf(L"Hello, from init layer!\r\n");
    printf(L"Press any key to continue...\r\n\r\n");
    get_key();

    return EFI_SUCCESS;

}
