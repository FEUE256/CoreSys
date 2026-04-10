// --------------------------------
// FÈUE File CoreSys Bootloader
// CyberBoot by FÈUE
// --------------------------------

#include <boot/headers.h>
#include <boot/typedef.h>
#include <stdbool.h>
#include <protocols/SFS.h>

// Return helper
void ret(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    clear_screen();
    
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Returned from EFI...\r\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Press any key to continue...\r\n");
    get_key();
    bmain_main(ImageHandle, SystemTable);
}

EFI_STATUS load_efi(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    CHAR16 *FilePath,
    EFI_HANDLE *OutImage
) {
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

    Status = Root->Open(
        Root,
        &File,
        FilePath,
        EFI_FILE_MODE_READ,
        0
    );
    if (EFI_ERROR(Status)) return Status;

    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData,
        InfoSize,
        (void**)&Info
    );
    if (EFI_ERROR(Status)) goto cleanup_file;

    Status = File->GetInfo(
        File,
        &gEfiFileInfoGuid,
        &InfoSize,
        Info
    );
    if (EFI_ERROR(Status)) goto cleanup_info;

    FileSize = (UINTN)Info->FileSize;

    if (FileSize <= 2) {
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

    /*
        IGNORE FIRST 2 BYTES WITHOUT MODIFYING FILE
    */
    UINT8 *Raw = (UINT8 *)Buffer;
    UINT8 *Adjusted = Raw + 2;
    UINTN AdjustedSize = FileSize - 2;

    EFI_HANDLE LoadedImage = NULL;

    Status = SystemTable->BootServices->LoadImage(
        FALSE,
        ImageHandle,
        NULL,
        (VOID *)Adjusted,
        AdjustedSize,
        &LoadedImage
    );

    if (EFI_ERROR(Status)) goto cleanup_buffer;

    *OutImage = LoadedImage;
    Status = EFI_SUCCESS;
    goto cleanup_info;

cleanup_buffer:
    if (Buffer) SystemTable->BootServices->FreePool(Buffer);

cleanup_info:
    if (Info) SystemTable->BootServices->FreePool(Info);

cleanup_file:
    if (File) File->Close(File);

    return Status;
}

EFI_STATUS jump(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE LoadedImage) {
    if (!LoadedImage || !SystemTable) return EFI_INVALID_PARAMETER;
    return SystemTable->BootServices->StartImage(LoadedImage, NULL, NULL);
}

// Unload an EFI image
EFI_STATUS unload_efi(EFI_HANDLE LoadedImage, EFI_SYSTEM_TABLE *SystemTable) {
    if (!LoadedImage || !SystemTable) return EFI_INVALID_PARAMETER;
    return SystemTable->BootServices->UnloadImage(LoadedImage);
}

// Boot helper: load, start, then return
EFI_STATUS boot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* name) {
    if (!SystemTable || !name) return EFI_INVALID_PARAMETER;

    EFI_STATUS Status;
    EFI_HANDLE LoadedImage = NULL;

    Status = load_efi(ImageHandle, SystemTable, name, &LoadedImage);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to load EFI file\r\n");
        return Status;
    }

    Status = jump(SystemTable, LoadedImage);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Failed to start EFI file\r\n");
    }

    unload_efi(LoadedImage, SystemTable);
    ret(ImageHandle, SystemTable);

    return Status;
}

// Main UEFI initialization
void cs_init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    if (!SystemTable) return;

    clear_screen();
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Jump to init layer\r\n");
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
    EFI_STATUS Status = boot(ImageHandle, SystemTable, L"\\EFI\\FEUE\\INIT.RE");

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

// Main HW initialization
void hw_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    if (!SystemTable) return;

    clear_screen();
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Jump to HW init layer\r\n");
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
    hw_imain(ImageHandle, SystemTable);
}
