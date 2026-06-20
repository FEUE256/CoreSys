// --------------------------------
// FÈUE File CoreSys Recovery OS Menu
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

// All std includes are in ../main.c
#include <boot/headers.h> // Project headers
#include <hw/std.h>        // Standard library for HW layer
#include <core/efi.h>        // Basic UEFI types

// =========================================================
// Show Menu Function
// =========================================================
void REQShowMenu(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout, UINTN selected) {
    clear_screen();

    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));

    cout->OutputString(cout, (CHAR16 *)L"CoreSys Recovery UEFI Version\r\n\r\n");
    cout->OutputString(cout, (CHAR16 *)L"Use the W/S/w/s keys to navigate, ENTER to confirm.\r\n\r\n");

    const CHAR16* options[] = {
        L"UEFI Shell",
        L"Go to main menu"
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

void req_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    UINTN selected = 0;
    EFI_INPUT_KEY key;
    const UINTN optionCount = 2;

    REQShowMenu(cout, selected);

    while (1) {
        if (cin->ReadKeyStroke(cin, &key) == EFI_SUCCESS) {
            switch (key.UnicodeChar) {
                case L'w': case L'W':
                    selected = (selected == 0) ? optionCount - 1 : selected - 1;
                    REQShowMenu(cout, selected);
                    break;
                case L's': case L'S':
                    selected = (selected + 1) % optionCount;
                    REQShowMenu(cout, selected);
                    break;
                case L'\r':  // Enter
                switch (selected) {
                    case 0: us_main(ImageHandle, SystemTable); break;
                    case 1: bmain_main(ImageHandle, SystemTable); break;
                }
                break;
            }
        }
    }
}
