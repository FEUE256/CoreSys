#pragma once
#include <core/efi.h> 

void clear_screen(void);
EFI_INPUT_KEY get_key(void);
void bmain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void cs_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
bool error(char *file, int line, const char *func, EFI_STATUS status, CHAR16 *fmt, ...);
bool printf(const CHAR16 *format, ...);
void init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);
void cs_init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void o_init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void hw_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
EFI_STATUS boot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* name);
EFI_STATUS unload_efi(EFI_HANDLE LoadedImage, EFI_SYSTEM_TABLE *SystemTable);
EFI_STATUS jump(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE LoadedImage);
EFI_STATUS load_efi(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, CHAR16 *FilePath, EFI_HANDLE *OutImage);
void ret(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
CHAR16* read_line(EFI_SYSTEM_TABLE *SystemTable);
EFI_STATUS EFIAPI hw_imain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void us_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void EFIAPI req_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void reboot(void);
void shutdown(void);
EFI_STATUS EFIAPI cb(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);