#pragma once 

#include <core/efi.h>         // Basic UEFI types
#include <core/efi_lib.h>     // Simple EFI lib
#include <core/stdio_efi.h>   // Includes Clear Screen

// =========================================================
// EFI reboot (cold)
// =========================================================
void reboot(void) {
    clear_screen();
    cout->OutputString(cout, L"\r\nRebooting...\r\n");
    rs->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
}

// =========================================================
// EFI shutdown
// =========================================================
void shutdown(void) {
    clear_screen();
    cout->OutputString(cout, L"\r\nShutting down...\r\n");
    rs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}
