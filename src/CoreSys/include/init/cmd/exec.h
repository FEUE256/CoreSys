#pragma once

#include <core/efi.h>
#include <core/efi_lib.h>
#include <project/power.h>
#include <protocols/SFS.h>

void fgets(CHAR16 *buffer, UINTN max_len);
void clear_screen();

void chelp() {
    printf(L"Commands: clear/cls (Clear the screen), help (Shows this text), exit (Quits the program), shutdown (Shuts the computer down), reboot (Reboots the computer), pwd (Print current dir name), run (runs the asm code in CoreSys/src/CoreSys/asm/code.s x86_64 ASM (asm.efi in UDP)), ps (Lists all proceess), ls (lists all files), whoami (Prints current user), uname (Prints spec)");
}


// -------------------------------
// Load EFI image
// -------------------------------
EFI_STATUS b_load_efi(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    CHAR16 *FilePath,
    EFI_HANDLE *OutImage
)
{
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
    EFI_FILE_PROTOCOL *Root = NULL, *File = NULL;
    EFI_STATUS Status;
    void *Buffer = NULL;
    EFI_FILE_INFO *Info = NULL;

    UINTN FileSize;
    UINTN InfoSize = sizeof(EFI_FILE_INFO) + 200;

    Status = SystemTable->BootServices->LocateProtocol(
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        (void**)&Fs
    );
    if (EFI_ERROR(Status)) return Status;

    Status = Fs->OpenVolume(Fs, &Root);
    if (EFI_ERROR(Status)) return Status;

    Status = Root->Open(Root, &File, FilePath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) return Status;

    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        InfoSize,
        (void**)&Info
    );
    if (EFI_ERROR(Status)) goto cleanup_file;

    Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, Info);
    if (EFI_ERROR(Status)) goto cleanup_info;

    FileSize = (UINTN)Info->FileSize;

    if (FileSize <= 2)
    {
        Status = EFI_INVALID_PARAMETER;
        goto cleanup_info;
    }

    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        FileSize,
        &Buffer
    );
    if (EFI_ERROR(Status)) goto cleanup_info;

    Status = File->Read(File, &FileSize, Buffer);
    if (EFI_ERROR(Status)) goto cleanup_buffer;

    UINT8 *Raw = (UINT8 *)Buffer;
    UINT8 *Adjusted = Raw + 2;
    UINTN AdjustedSize = FileSize - 2;

    EFI_HANDLE LoadedImage = NULL;

    Status = SystemTable->BootServices->LoadImage(
        FALSE,
        ImageHandle,
        NULL,
        Adjusted,
        AdjustedSize,
        &LoadedImage
    );

    if (EFI_ERROR(Status)) goto cleanup_buffer;

    *OutImage = LoadedImage;

cleanup_buffer:
    if (Buffer) SystemTable->BootServices->FreePool(Buffer);

cleanup_info:
    if (Info) SystemTable->BootServices->FreePool(Info);

cleanup_file:
    if (File) File->Close(File);

    return Status;
}

// -------------------------------
// Jump to image
// -------------------------------
EFI_STATUS bjump(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE LoadedImage)
{
    return SystemTable->BootServices->StartImage(LoadedImage, NULL, NULL);
}

// -------------------------------
// Unload EFI image
// -------------------------------
EFI_STATUS b_unload_efi(EFI_HANDLE LoadedImage, EFI_SYSTEM_TABLE *SystemTable)
{
    return SystemTable->BootServices->UnloadImage(LoadedImage);
}

// -------------------------------
// Boot helper
// -------------------------------
EFI_STATUS bboot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* name)
{
    EFI_HANDLE LoadedImage = NULL;

    EFI_STATUS Status = load_efi(ImageHandle, SystemTable, name, &LoadedImage);
    if (EFI_ERROR(Status))
        goto cleanup;

    Status = bjump(SystemTable, LoadedImage);

cleanup:
    printf(L"Return. Press any key...\r\n");
    get_key();

    if (LoadedImage)
        b_unload_efi(LoadedImage, SystemTable);

    return Status;
}



typedef void (*BIN_ENTRY)(void);

void cmd(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    clear_screen();
    CHAR16 input[256];

    UINTN currentAttr;
    gST->ConOut->QueryMode(gST->ConOut, gST->ConOut->Mode->Mode, NULL, &currentAttr);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);

    while (1) {

        printf(L"UDP/> ");

        fgets(input, sizeof(input));

        // Trim newline
        for (UINTN i = 0; input[i]; i++) {
            if (input[i] == L'\r' || input[i] == L'\n') {
                input[i] = 0;
                break;
            }
        }

        if (strncmp_u16(input, L"exit", 4) == 0) {
            bmain_main(ImageHandle, SystemTable);
        }

        else if (strncmp_u16(input, L"clear", 5) == 0 ||
                 strncmp_u16(input, L"cls", 3) == 0) {
            clear_screen();
        }

        else if (strncmp_u16(input, L"help", 4) == 0) {
            chelp();
        }

        else if (strncmp_u16(input, L"shutdown", 8) == 0) {
            shutdown();
        }

        else if (strncmp_u16(input, L"reboot", 6) == 0) {
            reboot();
        }

        else if (strncmp_u16(input, L"pwd", 3) == 0) {
            printf(L"UDP/\r\n");
        }

        else if (strncmp_u16(input, L"ls", 2) == 0) {
            printf(L"asm.s\r\n");
        }

        else if (strncmp_u16(input, L"ps", 2) == 0) {
            printf(L"/cmd\r\n");
            printf(L"ps\r\n");
        }

        else if (strncmp_u16(input, L"whoami", 6) == 0) {
            printf(L"CoreSys/root\r\n");
        }

        else if (strncmp_u16(input, L"uname", 5) == 0) {
            printf(L"CoreSys\r\n");
        }

        else if (strncmp_u16(input, L"run", 3) == 0) {
            bboot(image, gST, L"\\OWN\\C.RE");
        }

        else {
            printf(L"[Unknown command]\r\n");
        }
    }
}
