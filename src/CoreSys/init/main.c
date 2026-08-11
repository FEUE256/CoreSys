// -----------------------------------------
// FÈUE CoreSys Init
// INIT / OS entry point
// Namespace io.feue.coresys.init.imain
// -----------------------------------------

#include <core/efi.h>
#include <core/efi_lib.h>
#include <arch/x86_64.h>
#include <char/string_funcs.h>
#include <core/stdio_efi.h>
#include <project/codes.h>
#include <project/power.h>
#include <protocols/SFS.h>
#include <protocols/GOP.h>
#include <protocols/ACPI.h>
#include <project/compGUID.h>
#include <stdbool.h>
#include <kernel/mem.h>
#include <init/headers.h>
#include <API/CoreSys.h>

// ─── Globals ──────────────────────────────────────────────────────────────────

BOOT_INFO boot_info;

// ─── EFI context init ─────────────────────────────────────────────────────────

void init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    image = ImageHandle;
    st    = SystemTable;
    bs    = SystemTable->BootServices;
    rs    = SystemTable->RuntimeServices;

    gST   = SystemTable;
    gBS   = SystemTable->BootServices;
    gRS   = SystemTable->RuntimeServices;

    cout  = SystemTable->ConOut;
    cin   = SystemTable->ConIn;
}

// ─── Memory map ───────────────────────────────────────────────────────────────

void get_memory_mapl(EFI_SYSTEM_TABLE *SystemTable)
{
    UINTN            map_size        = 0;
    UINTN            map_key         = 0;
    UINTN            desc_size       = 0;
    UINT32           desc_version    = 0;
    EFI_STATUS       status;

    // First call: get required buffer size
    SystemTable->BootServices->GetMemoryMap(
        &map_size, NULL, &map_key, &desc_size, &desc_version
    );

    map_size += desc_size * 8; // headroom for descriptor churn

    EFI_MEMORY_DESCRIPTOR *map = NULL;

    status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, map_size, (VOID **)&map
    );
    if (EFI_ERROR(status))
    {
        printf(L"get_memory_map: AllocatePool failed: %llx\r\n", (unsigned long long)status);
        return;
    }

    status = SystemTable->BootServices->GetMemoryMap(
        &map_size, map, &map_key, &desc_size, &desc_version
    );
    if (EFI_ERROR(status))
    {
        printf(L"get_memory_map: GetMemoryMap failed: %llx\r\n", (unsigned long long)status);
        SystemTable->BootServices->FreePool(map);
        return;
    }

    boot_info.memory_entries  = 0;
    boot_info.installed_ram   = 0;
    boot_info.reserved_ram    = 0;

    UINTN entry_count = map_size / desc_size;

    for (UINTN i = 0; i < entry_count; i++)
    {
        EFI_MEMORY_DESCRIPTOR *desc =
            (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)map + i * desc_size);

        UINT64 region_bytes = desc->NumberOfPages * 4096ULL;

        if (boot_info.memory_entries < MAX_MEMORY_ENTRIES)
        {
            boot_info.memory_map[boot_info.memory_entries].base   = desc->PhysicalStart;
            boot_info.memory_map[boot_info.memory_entries].length = region_bytes;
            boot_info.memory_map[boot_info.memory_entries].type   = desc->Type;
            boot_info.memory_entries++;
        }

        switch (desc->Type)
        {
            case EfiConventionalMemory:
            case EfiLoaderCode:
            case EfiLoaderData:
            case EfiBootServicesCode:
            case EfiBootServicesData:
                boot_info.installed_ram += region_bytes;
                break;

            case EfiReservedMemoryType:
            case EfiUnusableMemory:
            case EfiACPIReclaimMemory:
            case EfiACPIMemoryNVS:
                boot_info.reserved_ram += region_bytes;
                break;

            case EfiMemoryMappedIO:
            case EfiMemoryMappedIOPortSpace:
            default:
                break;
        }
    }

    // TODO: replace with CPUID leaf 0x80000008 (EAX bits [7:0])
    boot_info.physical_address_width  = 40;
    boot_info.max_supported_memory    = 1ULL << boot_info.physical_address_width;

    SystemTable->BootServices->FreePool(map);
}

// ─── ACPI ─────────────────────────────────────────────────────────────────────

void get_acpi_info(EFI_SYSTEM_TABLE *SystemTable)
{
    boot_info.rsdp           = 0;
    boot_info.rsdt           = 0;
    boot_info.xsdt           = 0;
    boot_info.acpi_available = 0;
    boot_info.acpi_revision  = 0;

    for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++)
    {
        EFI_CONFIGURATION_TABLE *table = &SystemTable->ConfigurationTable[i];
        ACPI_RSDP *rsdp = (ACPI_RSDP *)table->VendorTable;

        bool is_acpi2 = memcmp(&table->VendorGuid, &gEfiAcpi20TableGuid, sizeof(EFI_GUID)) == 0;
        bool is_acpi1 = memcmp(&table->VendorGuid, &gEfiAcpi10TableGuid, sizeof(EFI_GUID)) == 0;

        if (!is_acpi2 && !is_acpi1)
            continue;

        boot_info.rsdp           = (uint64_t)rsdp;
        boot_info.acpi_revision  = rsdp->Revision;
        boot_info.acpi_available = 1;

        if (is_acpi2 && rsdp->Revision >= 2)
            boot_info.xsdt = rsdp->XsdtAddress;
        else
            boot_info.rsdt = rsdp->RsdtAddress;

        if (is_acpi2)
            break; // prefer ACPI 2.0 — stop on first match
    }
}

// ─── Framebuffer ──────────────────────────────────────────────────────────────

void get_framebuffer(void)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;

    boot_info.framebuffer      = 0;
    boot_info.framebuffer_size = 0;
    boot_info.width            = 0;
    boot_info.height           = 0;
    boot_info.pitch            = 0;
    boot_info.format           = 0;

    EFI_STATUS status = gST->BootServices->LocateProtocol(
        &gopGuid, NULL, (VOID **)&gop
    );
    if (EFI_ERROR(status))
    {
        printf(L"get_framebuffer: GOP not found: %llx\r\n", (unsigned long long)status);
        return;
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = gop->Mode->Info;

    boot_info.framebuffer      = gop->Mode->FrameBufferBase;
    boot_info.framebuffer_size = gop->Mode->FrameBufferSize;
    boot_info.width            = info->HorizontalResolution;
    boot_info.height           = info->VerticalResolution;
    boot_info.pitch            = info->PixelsPerScanLine;
    boot_info.format           = info->PixelFormat;
}

// ─── Write boot info to shared memory slots ───────────────────────────────────

void mem_write(int debug)
{
    get_memory_mapl(gST);
    get_acpi_info(gST);
    get_framebuffer();

    *(volatile uint8_t  *)KDI = (uint8_t)debug;
    *(volatile BOOT_INFO *)KSP = boot_info;
}

// ─── ExitBootServices ─────────────────────────────────────────────────────────

EFI_STATUS exit_boot_services(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    UINTN      map_size       = 0;
    UINTN      map_key        = 0;
    UINTN      desc_size      = 0;
    UINT32     desc_version   = 0;
    EFI_STATUS status;

    status = SystemTable->BootServices->GetMemoryMap(
        &map_size, NULL, &map_key, &desc_size, &desc_version
    );
    if (status != EFI_BUFFER_TOO_SMALL)
        return status;

    map_size += desc_size * 8;

    EFI_MEMORY_DESCRIPTOR *map = NULL;

    status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, map_size, (VOID **)&map
    );
    if (EFI_ERROR(status))
        return status;

    status = SystemTable->BootServices->GetMemoryMap(
        &map_size, map, &map_key, &desc_size, &desc_version
    );
    if (EFI_ERROR(status))
    {
        SystemTable->BootServices->FreePool(map);
        return status;
    }

    status = SystemTable->BootServices->ExitBootServices(ImageHandle, map_key);

    // NOTE: on success, BootServices are dead — FreePool is intentionally skipped.
    // On failure, we return the error; the caller should not retry without
    // re-fetching the memory map since the map key is stale after a failed exit.
    if (EFI_ERROR(status))
        SystemTable->BootServices->FreePool(map);

    return status;
}

// ─── Load EFI image from disk ─────────────────────────────────────────────────

EFI_STATUS load_efi(
    EFI_HANDLE        ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    CHAR16           *FilePath,
    EFI_HANDLE       *OutImage
)
{
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs   = NULL;
    EFI_FILE_PROTOCOL               *Root = NULL;
    EFI_FILE_PROTOCOL               *File = NULL;
    EFI_FILE_INFO                   *Info = NULL;
    void                            *Buffer = NULL;
    EFI_STATUS                       status;

    UINTN InfoSize = sizeof(EFI_FILE_INFO) + 200;

    status = SystemTable->BootServices->LocateProtocol(
        &gEfiSimpleFileSystemProtocolGuid, NULL, (void **)&Fs
    );
    if (EFI_ERROR(status)) return status;

    status = Fs->OpenVolume(Fs, &Root);
    if (EFI_ERROR(status)) return status;

    status = Root->Open(Root, &File, FilePath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) return status;

    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, InfoSize, (void **)&Info);
    if (EFI_ERROR(status)) goto cleanup_file;

    status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, Info);
    if (EFI_ERROR(status)) goto cleanup_info;

    UINTN FileSize = (UINTN)Info->FileSize;

    if (FileSize <= 2)
    {
        status = EFI_INVALID_PARAMETER;
        goto cleanup_info;
    }

    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, FileSize, &Buffer);
    if (EFI_ERROR(status)) goto cleanup_info;

    status = File->Read(File, &FileSize, Buffer);
    if (EFI_ERROR(status)) goto cleanup_buffer;

    // Skip 2-byte NE prefix written by the build system
    UINT8 *Payload     = (UINT8 *)Buffer + 2;
    UINTN  PayloadSize = FileSize - 2;

    EFI_HANDLE LoadedImage = NULL;

    status = SystemTable->BootServices->LoadImage(
        FALSE, ImageHandle, NULL, Payload, PayloadSize, &LoadedImage
    );
    if (EFI_ERROR(status)) goto cleanup_buffer;

    // Log kernel base/size for diagnostics
    EFI_GUID                   LipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *Lip     = NULL;

    if (!EFI_ERROR(SystemTable->BootServices->HandleProtocol(LoadedImage, &LipGuid, (void **)&Lip)))
    {
        printf(L"Kernel Base: 0x%llx\r\n", (unsigned long long)Lip->ImageBase);
        printf(L"Kernel Size: 0x%llx\r\n", (unsigned long long)Lip->ImageSize);
    }

    *OutImage = LoadedImage;

cleanup_buffer:
    if (Buffer) SystemTable->BootServices->FreePool(Buffer);
cleanup_info:
    if (Info)   SystemTable->BootServices->FreePool(Info);
cleanup_file:
    if (File)   File->Close(File);

    return status;
}

// ─── Jump to kernel entry point ───────────────────────────────────────────────

int kjump(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE LoadedImage)
{
    EFI_LOADED_IMAGE_PROTOCOL *Lip  = NULL;

    gBS->HandleProtocol(
        LoadedImage, &gEfiLoadedImageProtocolGuid, (void **)&Lip
    );

    IMAGE_DOS_HEADER      *Dos = (IMAGE_DOS_HEADER *)Lip->ImageBase;
    IMAGE_NT_HEADERS64_PART *Nt =
        (IMAGE_NT_HEADERS64_PART *)((UINT8 *)Lip->ImageBase + Dos->e_lfanew);

    void *EntryPoint = (UINT8 *)Lip->ImageBase + Nt->OptionalHeader.AddressOfEntryPoint;

    // C99/C11 compliant function-pointer recovery via union
    typedef int (*KernelEntry)(void);
    union { void *obj; KernelEntry fn; } cast;
    cast.obj = EntryPoint;

    EFI_STATUS status = exit_boot_services(image, SystemTable);
    if (EFI_ERROR(status))
    {
        printf(L"ExitBootServices failed: 0x%llx\r\n", (unsigned long long)status);
        while (1);
    }

    return cast.fn();
}

// ─── Unload image ─────────────────────────────────────────────────────────────

EFI_STATUS unload_efi(EFI_HANDLE LoadedImage, EFI_SYSTEM_TABLE *SystemTable)
{
    return SystemTable->BootServices->UnloadImage(LoadedImage);
}

// ─── Load, jump, unload ───────────────────────────────────────────────────────

EFI_STATUS kboot(
    EFI_HANDLE        ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    CHAR16           *path
)
{
    EFI_HANDLE LoadedImage = NULL;

    EFI_STATUS status = load_efi(ImageHandle, SystemTable, path, &LoadedImage);
    if (EFI_ERROR(status))
    {
        printf(L"kboot: load_efi failed: 0x%llx\r\n", (unsigned long long)status);
        goto cleanup;
    }

    kjump(SystemTable, LoadedImage); // does not return on success

cleanup:
    printf(L"Returned from kernel (press any key)\r\n");
    get_key();

    if (LoadedImage)
        unload_efi(LoadedImage, SystemTable);

    return status;
}

// ─── Kernel launch UI ─────────────────────────────────────────────────────────

void kernel(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, int debug)
{
    clear_screen();

    if (debug == 404)
        cout->OutputString(cout, L"Jump to Kernel to shutdown\r\n");
    else
        cout->OutputString(cout, L"Jump to Kernel\r\n");

    cout->OutputString(cout, L"Press ESC to return\r\n");

    UINTN      index;
    EFI_INPUT_KEY key;

    SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &index);
    SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

    if (key.ScanCode == SCANCODE_ESC)
    {
        imain_main(ImageHandle, SystemTable);
        return;
    }

    mem_write(debug);

    EFI_STATUS status = kboot(ImageHandle, SystemTable, L"\\EFI\\FEUE\\KERNEL.RE");
    printf(L"Kernel returned: 0x%llx\r\n", (unsigned long long)status);
}

// ─── Menu ─────────────────────────────────────────────────────────────────────

const CHAR16 *menu_options[] =
{
    L"CoreSys Kernel (Normal)",
    L"CoreSys Kernel (Debug)",
    L"CoreSys Kernel (Silent — not recommended)",
    L"Shutdown",
};

#define MENU_COUNT 4

void show_menu(UINTN selected)
{
    clear_screen();
    cout->SetAttribute(cout, EFI_TEXT_ATTR(EFI_BLACK, EFI_WHITE));
    cout->OutputString(cout, L"CoreSys Init UEFI\r\n\r\n");
    cout->OutputString(cout, L"W/S to navigate   ENTER to select\r\n\r\n");

    for (UINTN i = 0; i < MENU_COUNT; i++)
    {
        cout->OutputString(cout, (i == selected) ? L"> " : L"  ");
        cout->OutputString(cout, (CHAR16 *)menu_options[i]);
        cout->OutputString(cout, L"\r\n");
    }
}

// ─── Main menu loop ───────────────────────────────────────────────────────────

void imain_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    UINTN selected = 0;
    EFI_INPUT_KEY key;

    show_menu(selected);

    while (1)
    {
        if (cin->ReadKeyStroke(cin, &key) != EFI_SUCCESS)
            continue;

        switch (key.UnicodeChar)
        {
            case L'w':
            case L'W':
                selected = (selected == 0) ? MENU_COUNT - 1 : selected - 1;
                show_menu(selected);
                break;

            case L's':
            case L'S':
                selected = (selected + 1) % MENU_COUNT;
                show_menu(selected);
                break;

            case L'\r':
                switch (selected)
                {
                    case 0: kernel(ImageHandle, SystemTable, 0);   break;
                    case 1: kernel(ImageHandle, SystemTable, 1);   break;
                    case 2: kernel(ImageHandle, SystemTable, 2);   break;
                    case 3: kernel(ImageHandle, SystemTable, 404); break;
                }
                break;

            default:
                break;
        }
    }
}

// ─── Entry ────────────────────────────────────────────────────────────────────

EFI_STATUS EFIAPI imain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    init(ImageHandle, SystemTable);
    clear_screen();

    cs_logf(CS_LOG_INFO, u"CoreSys Init has been booted successfully");
    cs_logf(CS_LOG_INFO, u"Press any key to continue...");

    get_key();

    imain_main(ImageHandle, SystemTable);

    return EFI_SUCCESS;
}
