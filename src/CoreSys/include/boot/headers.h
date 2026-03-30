#pragma once
#include <core/efi.h> 

void clear_screen(void);
EFI_INPUT_KEY get_key(void);
void bmain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void cs_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
bool error(char *file, int line, const char *func, EFI_STATUS status, CHAR16 *fmt, ...);
