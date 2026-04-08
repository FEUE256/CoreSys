// --------------------------------
// FÈUE File CoreSys Bootloader
// CyberBoot by FÈUE
// --------------------------------

#include <boot/headers.h>
#include <boot/typedef.h>

#define MAX_INPUT 256

CHAR16* read_line(EFI_SYSTEM_TABLE *SystemTable) {
    static CHAR16 buffer[MAX_INPUT];
    UINTN pos = 0;

    EFI_INPUT_KEY key;
    UINTN index;

    while (1) {
        SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &index);
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

        if (key.UnicodeChar == L'\r') {
            buffer[pos] = L'\0';
            
            // Normalize double backslashes -> single backslash
            for (UINTN i = 0; buffer[i]; i++) {
                if (buffer[i] == L'\\' && buffer[i+1] == L'\\') {
                    for (UINTN j = i; buffer[j]; j++)
                        buffer[j] = buffer[j+1];
                }
            }

            SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");
            return buffer;
        }

        if (key.UnicodeChar == L'\b' && pos > 0) {
            pos--;
            SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\b \b");
            continue;
        }

        if (key.UnicodeChar != 0 && pos < MAX_INPUT - 1) {
            buffer[pos++] = key.UnicodeChar;
            CHAR16 out[2] = { key.UnicodeChar, L'\0' };
            SystemTable->ConOut->OutputString(SystemTable->ConOut, out);
        }
    }
}

void o_init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    clear_screen();
    
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Enter a the efi file you want to boot (Ex: \\\\EFI\\\\BOOT\\\\BOOTX64.EFI, Press enter when you are done!)\r\n");
    CHAR16* input = read_line(SystemTable);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Booting ");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, input);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"...\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press any ESC to exit...\r\n");
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
    EFI_STATUS Status = boot(ImageHandle, SystemTable, input);

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

    bmain_main(ImageHandle, SystemTable);
}
