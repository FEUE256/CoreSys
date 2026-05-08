// --------------------------------
// FÈUE File CoreSys Bootloader
// Contains code of the project
// CyberBoot by FÈUE
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

#include <init/cmd/exec.h> // cmd();

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
        L"Funcs",
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
                    case 2: efi_main(ImageHandle, SystemTable); break; // extern Function
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
    const UINTN optionCount = 5;

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
                    case 4: shutdown(); break;
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
