#pragma once

#include <core/efi.h>
#include <core/efi_lib.h>
#include <core/stdio_efi.h>
#include <boot/headers.h>
#include <project/power.h>

void sf(char *file, int line, const char *func, EFI_STATUS status, CHAR16 *fmt, ...) {
    printf(L"SYSTEM FAILURE!!!\n");
    printf(L"SYSTEM FAILURE!!!\n");
    printf(L"SYSTEM FAILURE!!!\n");

    error(file, line, func, status, fmt);

    printf(L"\r\n");
    printf(L"\r\nYour computer must reboot");
    printf(L"\r\nPress any key to reboot...");
    get_key();
    reboot();
}
