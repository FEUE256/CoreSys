#pragma once 

#include <core/efi.h>

extern EFI_SYSTEM_TABLE *gST;
extern EFI_RUNTIME_SERVICES *gRS;

void reboot() {
    gST->ConOut->OutputString(gST->ConOut, L"\r\nRebooting...\r\n");
    gRS->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
}

void shutdown() {
    gST->ConOut->OutputString(gST->ConOut, L"\r\nShutting down...\r\n");
    gRS->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}