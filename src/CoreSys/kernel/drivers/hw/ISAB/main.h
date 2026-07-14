// PCI/PCIe ISA Brige Driver (Approved by FÈUE in the CoreSys kernel hardware drivers collation)
#pragma once

#include <drivers/serial/main.h>
#include <drivers/pci/main.h>
#include <drivers/sf/main.h>
#include <drivers/task/main.h>

#include <kernel/mem.h>

#include <stdint.h>

typedef struct
{
    uint16_t vendor;
    uint16_t device;

    uint8_t bus;
    uint8_t dev;
    uint8_t fun;

    uint8_t revision;

    uint16_t command;
    uint16_t status;

    uint32_t pm_base;
    uint8_t acpi_enabled;

} ISAB_DEVICE;


ISAB_DEVICE isab;


/*
    Detect ISA Bridge
*/

int isab_detect(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    uint16_t vendor =
        pci_read16(
            bus,
            dev,
            fun,
            0x00
        );


    uint16_t device =
        pci_read16(
            bus,
            dev,
            fun,
            0x02
        );


    uint8_t class =
        pci_read8(
            bus,
            dev,
            fun,
            0x0B
        );


    uint8_t subclass =
        pci_read8(
            bus,
            dev,
            fun,
            0x0A
        );


    if(class != 0x06 ||
       subclass != 0x01)
    {
        return 0;
    }


    isab.vendor = vendor;
    isab.device = device;

    isab.bus = bus;
    isab.dev = dev;
    isab.fun = fun;


    isab.revision =
        pci_read8(
            bus,
            dev,
            fun,
            0x08
        );


    isab.command =
        pci_read16(
            bus,
            dev,
            fun,
            0x04
        );


    isab.status =
        pci_read16(
            bus,
            dev,
            fun,
            0x06
        );


    return 1;
}



/*
    Dump ISA Bridge information
*/

void isab_dump(void)
{

    kprintf(
        "========== ISA Bridge ==========\n"
    );


    kprintf("Vendor: ");
    kprint_u64(isab.vendor);

    kprintf("\nDevice: ");
    kprint_u64(isab.device);


    kprintf("\nRevision: ");
    kprint_u64(isab.revision);


    kprintf("\nCommand: ");
    kprint_u64(isab.command);


    kprintf("\nStatus: ");
    kprint_u64(isab.status);


    kprintf("\nLegacy I/O:\n");


    kprintf("Keyboard : 0x60-0x64\n");
    kprintf("RTC      : 0x70-0x71\n");
    kprintf("COM1     : 0x3F8\n");
    kprintf("COM2     : 0x2F8\n");
    kprintf("LPT1     : 0x378\n");


    kprintf(
        "================================\n"
    );
}



/*
    ISA IRQ table
*/

void isab_dump_irq(void)
{

    kprintf(
        "ISA IRQ Routing\n"
    );


    kprintf(
        "IRQ0  Timer\n"
    );

    kprintf(
        "IRQ1  Keyboard\n"
    );

    kprintf(
        "IRQ8  RTC\n"
    );

    kprintf(
        "IRQ12 PS/2 Mouse\n"
    );

    kprintf(
        "IRQ14 IDE Primary\n"
    );

    kprintf(
        "IRQ15 IDE Secondary\n"
    );
}



/*
    Legacy ports
*/

void isab_dump_ports(void)
{

    kprintf(
        "ISA Legacy Ports\n"
    );


    kprintf(
        "0x60 Keyboard Data\n"
    );

    kprintf(
        "0x64 Keyboard Command\n"
    );


    kprintf(
        "0x70 RTC Index\n"
    );


    kprintf(
        "0x3F8 COM1\n"
    );
}



/*
    Initialize ISAB
*/

void isab_dump_full()
{

    if(
        isab_detect(
            0,
            1,
            0
        )
    )
    {

        kprintf(
            "[ISAB] Intel ISA Bridge detected\n"
        );


        isab_dump();

        isab_dump_irq();

        isab_dump_ports();

    }
    else
    {

        kprintf(
            "[ISAB] ISA Bridge not found\n"
        );

    }

}
