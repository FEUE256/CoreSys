#pragma once

#include <core/efi.h>
#include <core/efi_lib.h>
#include <project/power.h>
#include <protocols/SFS.h>

void fgets(CHAR16 *buffer, UINTN max_len);
void clear_screen();

void chelp() {
    printf(L"Commands: clear/cls (Clear the screen), help (Shows this text), exit (Quits the program), shutdown (Shuts the computer down), reboot (Reboots the computer), pwd (Print current dir name), run (Runs PE2FMI), ps (Lists all proceess), whoami (Prints current user), uname (Prints spec), credits (Shows credits)\r\n");
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

// =========================================================
// Show Menu Function
// =========================================================
void ShowRMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"CoreSys Run UEFI Version\r\n\r\n");
    cout->OutputString(cout, (CHAR16 *)L"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"PE2FMI",
        L"Back to terminal"
    };
    const UINTN optionCount = sizeof(options) / sizeof(options[0]);

    for (UINTN i = 0; i < optionCount; i++) {
        if (i == selected) {
            cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));
            cout->OutputString(cout, (CHAR16 *)L"> ");
            cout->OutputString(cout, (CHAR16 *)options[i]);
            cout->OutputString(cout, (CHAR16 *)L"\r\n");
            cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
        } else {
            cout->OutputString(cout, (CHAR16 *)L"  ");
            cout->OutputString(cout, (CHAR16 *)options[i]);
            cout->OutputString(cout, (CHAR16 *)L"\r\n");
        }
    }
}

void rmain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    UINTN selected = 0;
    EFI_INPUT_KEY key;
    const UINTN optionCount = 2;

    ShowRMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    ShowRMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    ShowRMenu(cout, selected);
                    break;
                case L'\r':  // Enter
                switch (selected) {
                    case 0: bboot(ImageHandle, SystemTable, L"\\EFI\\PU\\C.RE"); break; // PU = Power User
                    case 1: return; break; // Back to main menu
                }
                break;
            }
        }
    }
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
            rmain_main(ImageHandle, SystemTable);
        }

        else if (strncmp_u16(input, L"credits", 7) == 0) {
            printf(L"Thanks to Queso Fuego (parts of code), elevatorguy (parts of code), Cyber::Boot (fork), Terry A. Davis (TempleOS), Ankit Kumar (Polaris OS), Neptune650 (Polaris OS), MishaTy (Polaris OS), redmine4404 (Polaris OS), AnalogFeelings (Polaris), 1010101001010101 (tinycrypt), chris-morrison (tinycrypt), mczraf (tinycrypt), Ipereira (tinycrypt), malsbat (tinycrypt), rob-brown (tinycrypt), haukepetersen (tinycrypt), mped-oticon (tinycrypt), thoh-ot (tinycrypt), daor-oti (tinycrypt), winnietwo (tinycrypt), sfblackl-intel (tinycrypt), every person on the EDK II team and many more for the inspiration and help in making this project possible! RIP Terry A. Davis!\r\n");
        }

        else {
            printf(L"[Unknown command]\r\n");
        }
    }
}
