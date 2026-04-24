// --------------------------------
// FÈUE File CoreSys C
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

#include <API/CoreSys.h> // CoreSys API

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

EFI_STATUS EFIAPI cmain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init(ImageHandle, SystemTable);
    clear_screen();
    fw_safe_exec();

    cs_logf(CS_LOG_INFO, u"CoreSys PE2FMI UEFI has been booted successfully");

    printf(L"This is meant for power users so they can create custom functions in CoreSys (Source Code: CoreSys/src/CoreSys/c/main.c)");
    printf(L"Press any key to continue...");
    get_key();

    return EFI_SUCCESS;
}
