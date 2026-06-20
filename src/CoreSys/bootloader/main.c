// --------------------------------
// FÈUE File CoreSys Bootloader
// Contains code of the project
// CyberBoot by FÈUE
// BOOTLOADER / OS
// --------------------------------

#include <core/efi.h>          // Basic UEFI types
#include <core/efi_lib.h>      // Simple EFI lib
#include <arch/x86_64.h>       // x86_64 definitions
#include <char/string_funcs.h> // String conversion functions
#include <core/stdio_efi.h>    // Custom printf for UEFI
#include <project/codes.h>     // Project codes/macros
#include <project/power.h>     // Power functions
#include <sf/main.c>           // System failure function
#include <stdbool.h>
#include <init/funcs.h>
#include <API/CoreSys.h>       // CoreSys API

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);
void cl(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

#include <init/cmd/exec.h> // cmd();

// -------------------------------
// Load EFI image
// -------------------------------
EFI_STATUS l_load_efi(
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
EFI_STATUS ljump(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE LoadedImage)
{
    return SystemTable->BootServices->StartImage(LoadedImage, NULL, NULL);
}

// -------------------------------
// Unload EFI image
// -------------------------------
EFI_STATUS l_unload_efi(EFI_HANDLE LoadedImage, EFI_SYSTEM_TABLE *SystemTable)
{
    return SystemTable->BootServices->UnloadImage(LoadedImage);
}

// -------------------------------
// Boot helper
// -------------------------------
EFI_STATUS lboot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* name)
{
    EFI_HANDLE LoadedImage = NULL;

    EFI_STATUS Status = l_load_efi(ImageHandle, SystemTable, name, &LoadedImage);
    if (EFI_ERROR(Status))
        goto cleanup;

    Status = ljump(SystemTable, LoadedImage);

cleanup:
    printf(L"Return. Press any key...\r\n");
    get_key();

    if (LoadedImage)
        l_unload_efi(LoadedImage, SystemTable);

    return Status;
}

void cl(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    if (!SystemTable) return;

    clear_screen();
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Jump to CoreSys Linux (via GRUB) layer\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"In the GRUB CLI run configfile (hd0,gpt1)/EFI/GRUB/GRUB.CFG to come to the GRUB mneu there you can press enter to boot CoreSys Linux!\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"QEMU GUI ONLY (Maybe on hardware (not expect to work)\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press the ESC key to exit...\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press any other key to continue...\r\n");

    // Wait for key
    EFI_INPUT_KEY key;
    UINTN index;

    SystemTable->BootServices->WaitForEvent(
        1,
        &SystemTable->ConIn->WaitForKey,
        &index
    );

    SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

    // ESC handling (ScanCode, not Unicode)
    if (key.ScanCode == SCANCODE_ESC) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"ESC pressed. Returning...\r\n");
        bmain_main(ImageHandle, SystemTable);
        return;
    }

    // Continue normally
    EFI_STATUS Status = lboot(ImageHandle, SystemTable, L"\\EFI\\GRUB\\GRUBX64.RE");

    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Boot failed\r\n");
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press any key to continue...\r\n");

        // wait again
        SystemTable->BootServices->WaitForEvent(
            1,
            &SystemTable->ConIn->WaitForKey,
            &index
        );
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

        bmain_main(ImageHandle, SystemTable);
    }

    // Cleanup (safe but currently unused)
    EFI_FILE_PROTOCOL *File = NULL;
    void *Buffer = NULL;
    EFI_FILE_INFO *Info = NULL;

    if (Buffer) SystemTable->BootServices->FreePool(Buffer);
    if (Info) SystemTable->BootServices->FreePool(Info);
    if (File) File->Close(File);
}

// START DONT NOT COPY TO funcs.h
void fgets(CHAR16 *buffer, UINTN max_len) {
    if (!buffer || max_len == 0) return;

    UINTN idx = 0;
    EFI_INPUT_KEY key;

    while (idx < max_len - 1) { // reservera plats för null
        // Vänta på tangenttryck
        if (gST->ConIn->ReadKeyStroke(gST->ConIn, &key) != EFI_SUCCESS) {
            continue;
        }

        // Enter avslutar raden
        if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            buffer[idx] = L'\0';
            // Skriv newline på skärmen
            gST->ConOut->OutputString(gST->ConOut, L"\r\n");
            break;
        }

        // Backspace
        if (key.UnicodeChar == CHAR_BACKSPACE && idx > 0) {
            idx--;
            gST->ConOut->OutputString(gST->ConOut, L"\b \b"); // ta bort tecken visuellt
            continue;
        }

        // Spara tecken i bufferten och skriv på skärmen
        buffer[idx++] = key.UnicodeChar;

        CHAR16 tmp[2] = { key.UnicodeChar, L'\0' };
        gST->ConOut->OutputString(gST->ConOut, tmp);
    }

    // Null-terminator om max_len uppnådd utan enter
    if (idx == max_len - 1) {
        buffer[idx] = L'\0';
    }
}

static void PrintHex(UINT8 *data, UINTN len)
{
    CHAR16 out[3];
    out[2] = L'\0';

    for (UINTN i = 0; i < len; i++) {
        UINT8 v = data[i];

        CHAR16 hi = (v >> 4) & 0xF;
        CHAR16 lo = v & 0xF;

        out[0] = (hi < 10) ? (L'0' + hi) : (L'a' + (hi - 10));
        out[1] = (lo < 10) ? (L'0' + lo) : (L'a' + (lo - 10));

        gST->ConOut->OutputString(gST->ConOut, out);
    }
}

UINTN wcslen(const CHAR16 *s);
extern void ShowFMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected);

EFI_STATUS sha256(void)
{
    clear_screen();

    CHAR16 input[256];
    UINT8 hash[32];

    gST->ConOut->OutputString(gST->ConOut, L"Write what you want to hash (UTF-16LE Encoding, 256 buffer): ");

    fgets(input, 256);

    // Convert CHAR16 string to raw bytes (UTF-16)
    UINTN len = wcslen(input) * sizeof(CHAR16);

    ComputeSha256(
        (const uint8_t *)input,
        len,
        hash
    );

    gST->ConOut->OutputString(gST->ConOut, L"\r\nSHA256: ");
    PrintHex(hash, 32);
    gST->ConOut->OutputString(gST->ConOut, L"\r\n");

    printf(L"\r\nPress any key to go back...\r\n");
    get_key();
    ShowFMenu(cout, 14);
    return EFI_SUCCESS;
}
// END

// =========================================================
// Show Menu Function
// =========================================================
void ShowFMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"CoreSys Functions UEFI Version\r\n\r\n");
    cout->OutputString(cout, (CHAR16 *)L"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"Set text mode",
        L"Set graphics mode",
        L"Test mouse",
        L"Test network",
        L"Print datetime",
        L"Read ESP",
        L"Print IO block partitions",
        L"Print memory map",
        L"Print config tables",
        L"Print ACPI tables",
        L"Print globle EFI variables",
        L"Change boot variables",
        L"CoreSys Terminal",
        L"GUI",
        L"Hashing (SHA256, CSC (CoreSys Cryptography))",
        L"Back to main menu"
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

void fmain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    UINTN selected = 0;
    EFI_INPUT_KEY key;
    const UINTN optionCount = 16;

    ShowFMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    ShowFMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    ShowFMenu(cout, selected);
                    break;
                case L'\r':  // Enter
                switch (selected) {
                    case 0: set_text_mode(); break;
                    case 1: set_graphics_mode(); break;
                    case 2: test_mouse(); break;
                    case 3: test_network(); break;
                    case 4: print_datetime(); break;
                    case 5: read_esp_files(); break;
                    case 6: print_block_io_partitions(); break;
                    case 7: print_memory_map(); break;
                    case 8: print_config_tables(); break;
                    case 9: print_acpi_tables(); break;
                    case 10: print_efi_global_variables(); break;
                    case 11: change_boot_variables(); break;
                    case 12: cmd(ImageHandle, SystemTable); break;
                    case 13: gmain(); break;
                    case 14: sha256(); break;
                    case 15: return; break; // Back to main menu
                }
                break;
            }
        }
    }
}

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

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    fmain_main(ImageHandle, SystemTable);

    bmain_main(ImageHandle, SystemTable);
    return EFI_SUCCESS;
}

void ShowCSMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected);

EFI_STATUS cr() {
    clear_screen();
    printf(L"Thanks to Queso Fuego (parts of code), elevatorguy (parts of code), Cyber::Boot (fork), Terry A. Davis (TempleOS), Ankit Kumar (Polaris OS), Neptune650 (Polaris OS), MishaTy (Polaris OS), redmine4404 (Polaris OS), AnalogFeelings (Polaris), 1010101001010101 (tinycrypt), chris-morrison (tinycrypt), mczraf (tinycrypt), Ipereira (tinycrypt), malsbat (tinycrypt), rob-brown (tinycrypt), haukepetersen (tinycrypt), mped-oticon (tinycrypt), thoh-ot (tinycrypt), daor-oti (tinycrypt), winnietwo (tinycrypt), sfblackl-intel (tinycrypt), every person on the EDK II team and many more for the inspiration and help in making this project possible! RIP Terry A. Davis!\r\n");
    printf(L"\r\nPress any key to go back...\r\n");
    get_key();
    ShowCSMenu(cout, 1);
    return EFI_SUCCESS;
}

// =========================================================
// Show CS Menu Function
// =========================================================
void ShowCSMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"CoreSys\r\n");
    cout->OutputString(cout, (CHAR16 *)L"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"CoreSys OS",
        L"Credits",
        L"Hardware Version",
        L"Back to Main Menu"
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

void cs_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    UINTN selected = 0;
    EFI_INPUT_KEY key;
    const UINTN optionCount = 4;

    ShowCSMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    ShowCSMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    ShowCSMenu(cout, selected);
                    break;
                case L'\r':  // Enter
                switch (selected) {
                    case 0: cs_init(ImageHandle, SystemTable); break; // extern Function
                    case 1: cr(); break; // extern Function
                    case 2: hw_main(ImageHandle, SystemTable); break; // extern Function
                    case 3: bmain_main(ImageHandle, SystemTable); break;
                }
                break;
            }
        }
    }
}

// =========================================================
// Show O Menu Function
// =========================================================
void ShowOMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"Other OS\r\n");
    cout->OutputString(cout, (CHAR16 *)L"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"Other OS",
        L"Cyber::Boot",
        L"CoreSys Linux",
        L"Back to Main Menu"
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

void o_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    UINTN selected = 0;
    EFI_INPUT_KEY key;
    const UINTN optionCount = 4;

    ShowOMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    ShowOMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    ShowOMenu(cout, selected);
                    break;
                case L'\r':  // Enter
                switch (selected) {
                    case 0: o_init(ImageHandle, SystemTable); break; // extern Function
                    case 1: cb(ImageHandle, SystemTable); break; // extern Function
                    case 2: cl(ImageHandle, SystemTable); break;
                    case 3: bmain_main(ImageHandle, SystemTable); break;
                }
                break;
            }
        }
    }
}

// =========================================================
// Show Menu Function
// =========================================================
void ShowMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cs_logf(CS_LOG_INFO, u"CoreSys Bootloader UEFI Version");
    cs_logf(CS_LOG_INFO, u"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"CoreSys OS",
        L"Other OS",
        L"Recovery",
        L"Exit",
        L"Extra functions",
        L"Shutdown"
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

void bmain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    UINTN selected = 0;
    EFI_INPUT_KEY key;
    const UINTN optionCount = 6;

    ShowMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    ShowMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    ShowMenu(cout, selected);
                    break;
                case L'\r':  // Enter
                switch (selected) {
                    case 0: cs_main(ImageHandle, SystemTable); break;
                    case 1: o_main(ImageHandle, SystemTable); break;
                    case 2: req_main(ImageHandle, SystemTable); break;
                    case 3: ExitApp(ImageHandle, SystemTable); break; // Exit
                    case 4: efi_main(ImageHandle, SystemTable); break; // Extra functions 
                    case 5: shutdown(); break;
                }
                break;
            }
        }
    }
}

EFI_STATUS EFIAPI bmain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    cs_logf(CS_LOG_INFO, u"CoreSys UEFI has been booted successfully.");
    cs_logf(CS_LOG_INFO, u"Press any key to continue...");

    get_key();

    bmain_main(ImageHandle, SystemTable);

    return EFI_SUCCESS;
}
