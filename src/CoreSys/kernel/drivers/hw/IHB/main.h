// PCI/PCIe Intel Host Brige Driver (Approved by FÈUE in the CoreSys kernel hardware drivers collation)
#pragma once

#include <drivers/serial/main.h>
#include <drivers/pci/main.h>
#include <drivers/sf/main.h>

#include <kernel/mem.h>

uint16_t ihb_get_vendor_id(void)
{
    return ihb_pci_read32(0, 0, 0, 0x00) & 0xFFFF;
}

uint16_t ihb_get_device_id(void)
{
    return (ihb_pci_read32(0, 0, 0, 0x00) >> 16) & 0xFFFF;
}

uint8_t ihb_get_revision_id(void)
{
    return ihb_pci_read32(0, 0, 0, 0x08) & 0xFF;
}

uint8_t ihb_get_programming_interface(void)
{
    return (ihb_pci_read32(0, 0, 0, 0x08) >> 8) & 0xFF;
}

uint8_t ihb_get_subclass(void)
{
    return (ihb_pci_read32(0, 0, 0, 0x08) >> 16) & 0xFF;
}

uint8_t ihb_get_class_code(void)
{
    return (ihb_pci_read32(0, 0, 0, 0x08) >> 24) & 0xFF;
}

const char *ihb_detect_chipset_model(void)
{
    uint16_t vendor = ihb_get_vendor_id();
    uint16_t device = ihb_get_device_id();

    if (vendor == 0x8086)
    {
        switch (device)
        {
            case 0x1237: return "Intel 440FX";
            case 0x29C0: return "Intel Q35";
            case 0x7190: return "Intel 440BX";
            default:     return "Unknown Intel Chipset";
        }
    }

    return "Unknown Chipset";
}

const char *ihb_detect_chipset_generation(void)
{
    uint16_t vendor = ihb_get_vendor_id();
    uint16_t device = ihb_get_device_id();

    if (vendor != 0x8086)
        return "Unknown";

    switch (device)
    {
        case 0x1237:
            return "440FX Generation";

        case 0x7190:
            return "440BX Generation";

        case 0x29C0:
            return "Q35 Generation";

        default:
            return "Unknown Generation";
    }
}

void ihb_general_print(void)
{
    kprintf("========== Intel Host Bridge ==========\n");

    kprintf("Vendor ID           : ");
    kprint_u64(ihb_get_vendor_id());
    kprintf("\n");

    kprintf("Device ID           : ");
    kprint_u64(ihb_get_device_id());
    kprintf("\n");

    kprintf("Revision ID         : ");
    kprint_u64(ihb_get_revision_id());
    kprintf("\n");

    kprintf("Class Code          : ");
    kprint_u64(ihb_get_class_code());
    kprintf("\n");

    kprintf("Subclass            : ");
    kprint_u64(ihb_get_subclass());
    kprintf("\n");

    kprintf("Programming IF      : ");
    kprint_u64(ihb_get_programming_interface());
    kprintf("\n");

    kprintf("Chipset Model       : %s\n", ihb_detect_chipset_model());
    kprintf("Chipset Generation  : %s\n", ihb_detect_chipset_generation());

    kprintf("=======================================\n");
}

// SECTION 1 END 

void ihb_pci_conf_print(void) {
    pci_dump_config(0,0,0);

    pci_detect_capabilities(0,0,0);

    pci_detect_extended_capabilities(0,0,0);

    kprintf("Command: ");
    kprint_u64(pci_read_command(0,0,0));

    kprintf("\nStatus: ");
    kprint_u64(pci_read_status(0,0,0));
    kprintf("\n");
}

// SECTION 2 END



/*
    Get boot information
*/

static BOOT_INFO *ihb_get_boot_info(void)
{
    return BOOT_INFO_PTR;
}



/*
    1. Detect installed RAM
*/

uint64_t ihb_detect_installed_ram(void)
{
    BOOT_INFO *boot = ihb_get_boot_info();


    if (!boot)
        return 0;


    uint64_t total = 0;


    if (boot->memory_entries > MAX_MEMORY_ENTRIES)
        boot->memory_entries = MAX_MEMORY_ENTRIES;



    for (uint64_t i = 0;
         i < boot->memory_entries;
         i++)
    {

        if (boot->memory_map[i].type == 7)
        {
            total +=
                boot->memory_map[i].length;
        }
    }


    boot->installed_ram = total;


    return total;
}



/*
    2. Read memory map
*/

void ihb_read_memory_map(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    kprintf("Memory Map:\n");


    for (uint64_t i = 0;
         i < boot->memory_entries;
         i++)
    {

        kprintf("Region ");

        kprint_u64(i);


        kprintf(" Base=");

        kprint_u64(
            boot->memory_map[i].base
        );


        kprintf(" Size=");

        kprint_u64(
            boot->memory_map[i].length
        );


        kprintf(" Type=");

        kprint_u64(
            boot->memory_map[i].type
        );


        kprintf("\n");
    }
}



/*
    3. Detect reserved memory
*/

uint64_t ihb_detect_reserved_memory(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    uint64_t reserved = 0;


    for (uint64_t i = 0;
         i < boot->memory_entries;
         i++)
    {

        if (boot->memory_map[i].type != 7)
        {
            reserved +=
                boot->memory_map[i].length;
        }

    }


    boot->reserved_ram = reserved;


    return reserved;
}



/*
    4. MMIO
*/

void ihb_detect_mmio_regions(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    kprintf("MMIO Regions:\n");


    for(uint64_t i = 0;
        i < boot->memory_entries;
        i++)
    {

        if(boot->memory_map[i].type == 11)
        {
            kprintf("Base=");

            kprint_u64(
                boot->memory_map[i].base
            );


            kprintf(" Size=");

            kprint_u64(
                boot->memory_map[i].length
            );


            kprintf("\n");
        }
    }
}



/*
    5. PCI windows
*/

void ihb_detect_pci_memory_windows(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    kprintf("PCI Memory Windows:\n");


    for(uint64_t i = 0;
        i < boot->memory_entries;
        i++)
    {

        uint64_t start =
            boot->memory_map[i].base;


        uint64_t end =
            start +
            boot->memory_map[i].length;



        if(start >= 0x80000000)
        {
            kprintf("PCI ");

            kprint_u64(start);

            kprintf(" - ");

            kprint_u64(end);

            kprintf("\n");
        }
    }
}



/*
    6. RAM holes
*/

void ihb_detect_ram_holes(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    kprintf("RAM holes:\n");


    uint64_t last_end = 0;



    for(uint64_t i = 0;
        i < boot->memory_entries;
        i++)
    {

        uint64_t start =
            boot->memory_map[i].base;



        if(start > last_end)
        {
            kprintf("Hole ");

            kprint_u64(last_end);

            kprintf(" - ");

            kprint_u64(start);

            kprintf("\n");
        }



        last_end =
            start +
            boot->memory_map[i].length;
    }
}



/*
    7. Physical address width
*/

uint64_t ihb_get_physical_address_width(void)
{
    uint32_t eax;


    asm volatile(
        "cpuid"
        : "=a"(eax)
        : "a"(0x80000008)
        : "ebx",
          "ecx",
          "edx"
    );


    uint64_t width =
        eax & 0xff;



    ihb_get_boot_info()
        ->physical_address_width =
        width;



    return width;
}



/*
    8. Maximum memory
*/

uint64_t ihb_get_supported_memory_size(void)
{
    uint64_t bits =
        ihb_get_physical_address_width();



    uint64_t max =
        1ULL << bits;



    ihb_get_boot_info()
        ->max_supported_memory =
        max;



    return max;
}



/*
    Print
*/

void ihb_memory_print(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();



    ihb_detect_installed_ram();

    ihb_detect_reserved_memory();

    ihb_get_physical_address_width();

    ihb_get_supported_memory_size();



    kprintf(
        "========== Memory Information ==========\n"
    );


    kprintf("Installed RAM: ");
    kprint_u64(
        boot->installed_ram
    );
    kprintf(" bytes\n");


    kprintf("Reserved RAM: ");
    kprint_u64(
        boot->reserved_ram
    );
    kprintf(" bytes\n");


    kprintf("Physical Address Width: ");
    kprint_u64(
        boot->physical_address_width
    );
    kprintf(" bits\n");


    kprintf("Maximum Supported Memory: ");
    kprint_u64(
        boot->max_supported_memory
    );
    kprintf(" bytes\n");



    ihb_read_memory_map();

    ihb_detect_mmio_regions();

    ihb_detect_pci_memory_windows();

    ihb_detect_ram_holes();



    kprintf(
        "========================================\n"
    );
}

// Section 4 end

/*
    BIOS / UEFI configuration memory
*/

UINT64 ihb_detect_uefi_configuration(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    UINT64 size = 0;


    for(UINT64 i = 0;
        i < boot->memory_entries;
        i++)
    {

        /*
            EFI Runtime Services Code/Data
            EFI ACPI Memory
        */

        if(
            boot->memory_map[i].type == EfiRuntimeServicesCode ||
            boot->memory_map[i].type == EfiRuntimeServicesData
        )
        {
            size +=
                boot->memory_map[i].length;
        }
    }


    return size;
}



/*
    Firmware reserved memory regions
*/

UINT64 ihb_detect_firmware_memory_regions(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    UINT64 size = 0;


    for(UINT64 i = 0;
        i < boot->memory_entries;
        i++)
    {

        switch(
            boot->memory_map[i].type
        )
        {

            /*
                Firmware areas
            */

            case EfiReservedMemoryType:

            case EfiLoaderCode:

            case EfiLoaderData:

            case EfiBootServicesCode:

            case EfiBootServicesData:

                size +=
                    boot->memory_map[i].length;

                break;
        }
    }


    return size;
}



/*
    ACPI memory locations
*/

UINT64 ihb_detect_acpi_memory(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    UINT64 size = 0;


    for(UINT64 i = 0;
        i < boot->memory_entries;
        i++)
    {

        if(
            boot->memory_map[i].type ==
            EfiACPIReclaimMemory
        )
        {
            size +=
                boot->memory_map[i].length;
        }



        if(
            boot->memory_map[i].type ==
            EfiACPIMemoryNVS
        )
        {
            size +=
                boot->memory_map[i].length;
        }

    }


    return size;
}

void ihb_uefi_conf_print() {
    kprintf("========== UEFI/BIOS Conf ==========\n");
    
    kprintf("UEFI Configuration Memory: ");
    kprint_u64(
        ihb_detect_uefi_configuration()
    );
    kprintf(" bytes\n");


    kprintf("Firmware Reserved Memory: ");
    kprint_u64(
        ihb_detect_firmware_memory_regions()
    );
    kprintf(" bytes\n");


    kprintf("ACPI Memory: ");
    kprint_u64(
        ihb_detect_acpi_memory()
    );
    kprintf(" bytes\n");

    kprintf(
        "========================================\n"
    );
}

// SECTION Start other

void ihb_dump_pci_windows(void)
{
    kprintf("========== PCI Memory Windows ==========\n");


    BOOT_INFO *boot = ihb_get_boot_info();


    for(UINT64 i = 0; i < boot->memory_entries; i++)
    {
        UINT64 base =
            boot->memory_map[i].base;

        UINT64 limit =
            base +
            boot->memory_map[i].length;


        /*
            PCI MMIO normally above 0x80000000
        */

        if(base >= 0x80000000)
        {
            kprintf("PCI Window: ");

            kprint_u64(base);

            kprintf(" - ");

            kprint_u64(limit);

            kprintf("\n");
        }
    }


    kprintf("========================================\n");
}

void ihb_dump_memory_controller(void)
{
    kprintf("========== Memory Controller ==========\n");


    /*
        Intel Host Bridge
        Usually bus 0 dev 0 func 0
    */


    uint16_t vendor =
        pci_read16(
            0,
            0,
            0,
            0x00
        );


    uint16_t device =
        pci_read16(
            0,
            0,
            0,
            0x02
        );


    kprintf("Vendor: ");
    kprint_u64(vendor);

    kprintf("\nDevice: ");
    kprint_u64(device);


    kprintf("\nMemory Controller Registers:\n");


    for(uint16_t reg = 0x40;
        reg < 0x100;
        reg += 4)
    {
        uint32_t value =
            pci_read32(
                0,
                0,
                0,
                reg
            );


        kprintf("0x");

        kprint_u64(reg);

        kprintf(": ");

        kprint_u64(value);

        kprintf("\n");
    }


    kprintf("========================================\n");
}

void pci_dump_windows(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{

    if(!pci_is_bridge(bus,dev,fun))
    {
        kprintf("Not PCI Bridge\n");
        return;
    }


    kprintf("PCI Bridge Windows\n");


    kprintf("Memory Base: ");
    kprint_u64(
        pci_get_memory_base(
            bus,dev,fun
        )
    );


    kprintf("\nMemory Limit: ");
    kprint_u64(
        pci_get_memory_limit(
            bus,dev,fun
        )
    );


    kprintf("\n");
}

void ihb_dump_bridge_configuration(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    kprintf("========== PCI Bridge Configuration ==========\n");


    uint16_t vendor =
        pci_read16(bus,dev,fun,0x00);


    uint16_t device =
        pci_read16(bus,dev,fun,0x02);


    uint8_t class =
        pci_read8(bus,dev,fun,0x0B);


    uint8_t subclass =
        pci_read8(bus,dev,fun,0x0A);



    kprintf("Vendor: ");
    kprint_u64(vendor);


    kprintf("\nDevice: ");
    kprint_u64(device);


    kprintf("\nClass: ");
    kprint_u64(class);


    kprintf("\nSubclass: ");
    kprint_u64(subclass);



    kprintf("\nBus numbers: ");


    uint32_t buses =
        pci_read32(
            bus,
            dev,
            fun,
            0x18
        );


    kprint_u64(buses);

    pci_dump_windows(bus, dev, fun);

    kprintf("\n===============================================\n");
}

void ihb_verify_firmware_setup(void)
{
    BOOT_INFO *boot =
        ihb_get_boot_info();


    kprintf("========== Firmware Verification ==========\n");


    if(boot->memory_entries == 0)
    {
        kprintf("[FAIL] No memory map\n");
        return;
    }


    if(boot->installed_ram == 0)
    {
        kprintf("[FAIL] No RAM detected\n");
        return;
    }


    if(boot->physical_address_width < 32)
    {
        kprintf("[WARN] Low physical address width\n");
    }


    kprintf("[OK] Memory map valid\n");
    kprintf("[OK] RAM detected\n");


    kprintf("===========================================\n");
}

void ihb_detect_hardware_misconfiguration(void)
{
    kprintf("========== Hardware Checks ==========\n");


    BOOT_INFO *boot =
        ihb_get_boot_info();



    if(boot->installed_ram < 64 * 1024 * 1024)
    {
        kprintf("[WARN] Low RAM\n");
    }



    if(boot->memory_entries >= MAX_MEMORY_ENTRIES)
    {
        kprintf("[WARN] Memory map full\n");
    }



    if(boot->physical_address_width < 36)
    {
        kprintf("[WARN] Limited address space\n");
    }



    kprintf("[OK] Hardware configuration scan complete\n");


    kprintf("=====================================\n");
}
