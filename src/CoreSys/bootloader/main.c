// --------------------------------
// FÈUE File CoreSys Bootloader
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
#include <sf/main.c>          // System failure function
#include <stdbool.h>

void cs_init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
void bmain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

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
    const UINTN optionCount = 2;

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
                    case 1: bmain_main(ImageHandle, SystemTable); break;
                }
                break;
            }
        }
    }
}

void o_main(void) {
        clear_screen();
        //printf(u"WIP...\r\n");
        //printf(u"Press any key to reboot...\r\n");
        EFI_STATUS status = EFI_SYSTEM_FAIL;
        sf(__FILE__, __LINE__, __func__, status, L"LBA Failure\n", L"0x010200000000010000010001000306080000000000000000000000000101\n");
}

// =========================================================
// Show Menu Function
// =========================================================
void ShowMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"CoreSys Bootloader UEFI Version\r\n\r\n");
    cout->OutputString(cout, (CHAR16 *)L"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"CoreSys OS",
        L"Others",
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
    const UINTN optionCount = 3;

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
                    case 1: o_main(); break;
                    case 2: shutdown(); break;
                }
                break;
            }
        }
    }
}

EFI_STATUS EFIAPI bmain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();

    bmain_main(ImageHandle, SystemTable);

    return EFI_SUCCESS;
}
