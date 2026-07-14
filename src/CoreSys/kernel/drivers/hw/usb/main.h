// PCI/PCIe Universal Serial Bus Driver (Approved by FÈUE in the CoreSys kernel hardware drivers collation)
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

    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;

    uint64_t mmio;

    uint8_t detected;

} USB_PCI_CONTROLLER;


USB_PCI_CONTROLLER usb_pci;


/*
    Get USB controller MMIO BAR
*/

uint64_t usb_get_mmio(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    uint32_t bar0 =
        pci_read32(
            bus,
            dev,
            fun,
            0x10
        );


    /*
        Memory BAR
    */

    if((bar0 & 0x1) == 0)
    {
        return (uint64_t)(bar0 & 0xFFFFFFF0);
    }


    return 0;
}



/*
    Detect USB PCI Controller
*/

int usb_pci_detect(void)
{

    for(uint16_t bus = 0; bus < 256; bus++)
    {
        for(uint8_t dev = 0; dev < 32; dev++)
        {
            for(uint8_t fun = 0; fun < 8; fun++)
            {

                uint16_t vendor =
                    pci_read16(
                        bus,
                        dev,
                        fun,
                        0x00
                    );


                if(vendor == 0xFFFF)
                    continue;


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


                uint8_t prog_if =
                    pci_read8(
                        bus,
                        dev,
                        fun,
                        0x09
                    );


                /*
                    USB Controller
                    Class: 0x0C
                    Subclass: 0x03
                */

                if(class == 0x0C &&
                   subclass == 0x03)
                {

                    usb_pci.vendor = vendor;

                    usb_pci.device =
                        pci_read16(
                            bus,
                            dev,
                            fun,
                            0x02
                        );


                    usb_pci.bus = bus;
                    usb_pci.dev = dev;
                    usb_pci.fun = fun;

                    usb_pci.class_code = class;
                    usb_pci.subclass = subclass;
                    usb_pci.prog_if = prog_if;


                    usb_pci.mmio =
                        usb_get_mmio(
                            bus,
                            dev,
                            fun
                        );


                    usb_pci.detected = 1;


                    return 1;
                }
            }
        }
    }


    return 0;
}



/*
    Dump USB PCI information
*/

void usb_pci_dump(void)
{

    if(!usb_pci.detected)
    {
        kprintf(
            "[USB] No USB controller found\n"
        );

        return;
    }


    kprintf(
        "========== USB PCI ==========\n"
    );


    kprintf(
        "Vendor: "
    );

    kprint_u64(
        usb_pci.vendor
    );


    kprintf(
        "\nDevice: "
    );

    kprint_u64(
        usb_pci.device
    );


    kprintf(
        "\nBus: "
    );

    kprint_u64(
        usb_pci.bus
    );


    kprintf(
        "\nDevice: "
    );

    kprint_u64(
        usb_pci.dev
    );


    kprintf(
        "\nFunction: "
    );

    kprint_u64(
        usb_pci.fun
    );


    kprintf(
        "\nClass: "
    );

    kprint_u64(
        usb_pci.class_code
    );


    kprintf(
        "\nSubclass: "
    );

    kprint_u64(
        usb_pci.subclass
    );


    kprintf(
        "\nProgIF: "
    );

    kprint_u64(
        usb_pci.prog_if
    );


    kprintf(
        "\nMMIO: 0x"
    );

    kprint_u64(
        usb_pci.mmio
    );


    kprintf(
        "\n=============================\n"
    );

}

void usb_general_print() {
    if(usb_pci_detect())
    {
        usb_pci_dump();
    }
    else
    {
        kprintf("[USB] Controller missing\n");
    }
}

// SECTION 1 END

static uint16_t uhci_io_base = 0;


/*
    Get UHCI I/O Base Address

    UHCI uses PCI BAR4 as I/O space
*/
uint16_t usb_uhci_get_io_base(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    uint32_t bar =
        pci_read32(
            bus,
            dev,
            fun,
            0x20
        );


    /*
        Check if BAR is I/O space
    */
    if (!(bar & 0x1))
    {
        kprintf("[UHCI] BAR is not I/O space\n");
        return 0;
    }


    return (uint16_t)(bar & ~0x3);
}



/*
    Reset UHCI Controller
*/
void usb_uhci_reset(void)
{
    if (!uhci_io_base)
        return;


    kprintf("[UHCI] Reset\n");


    /*
        USBCMD
        Bit 1 = Host Controller Reset
    */
    outw(
        uhci_io_base + 0x00,
        0x0002
    );


    for (volatile int i = 0; i < 100000; i++)
    {
        __asm__ volatile("nop");
    }


    outw(
        uhci_io_base + 0x00,
        0x0000
    );
}



/*
    Start UHCI Controller
*/
void usb_uhci_start(void)
{
    if (!uhci_io_base)
        return;


    kprintf("[UHCI] Start\n");


    /*
        Enable interrupts
        Clear status
    */

    outw(
        uhci_io_base + 0x02,
        0xFFFF
    );


    /*
        Enable USB interrupts
    */

    outw(
        uhci_io_base + 0x04,
        0x000F
    );


    /*
        Run controller
        USBCMD bit 0 = Run
    */

    uint16_t cmd =
        inw(
            uhci_io_base + 0x00
        );


    cmd |= 0x0001;


    outw(
        uhci_io_base + 0x00,
        cmd
    );
}



/*
    Dump UHCI Registers
*/
void usb_uhci_dump(void)
{
    if (!uhci_io_base)
    {
        kprintf("[UHCI] Not initialized\n");
        return;
    }


    kprintf("========== UHCI ==========\n");


    kprintf("IO Base: ");
    kprint_u64(uhci_io_base);


    kprintf("\nUSBCMD: ");
    kprint_u64(
        inw(uhci_io_base + 0x00)
    );


    kprintf("\nUSBSTS: ");
    kprint_u64(
        inw(uhci_io_base + 0x02)
    );


    kprintf("\nUSBINTR: ");
    kprint_u64(
        inw(uhci_io_base + 0x04)
    );


    kprintf("\nFRNUM: ");
    kprint_u64(
        inw(uhci_io_base + 0x06)
    );


    kprintf("\nPORT1: ");
    kprint_u64(
        inw(uhci_io_base + 0x10)
    );


    kprintf("\nPORT2: ");
    kprint_u64(
        inw(uhci_io_base + 0x12)
    );


    kprintf("\n==========================\n");
}



#define UHCI_FRAME_COUNT 1024


#define UHCI_USBCMD      0x00
#define UHCI_USBSTS      0x02
#define UHCI_FRNUM       0x06
#define UHCI_FLBASEADD   0x08
#define UHCI_PORTSC1     0x10


#define UHCI_PORT_ENABLE 0x0004
#define UHCI_PORT_CONN   0x0001
#define UHCI_PORT_RESET  0x0200


static uint32_t *uhci_frame_list = 0;


/*
    Allocate UHCI frame list
*/
void usb_uhci_alloc_frame_list(void)
{
    /*
        1024 * 4 bytes
    */

    uhci_frame_list =
        (uint32_t*)kmalloc(
            UHCI_FRAME_COUNT * sizeof(uint32_t)
        );


    if (!uhci_frame_list)
    {
        kprintf("[UHCI] Frame list allocation failed\n");
        return;
    }


    for(int i = 0; i < UHCI_FRAME_COUNT; i++)
    {
        /*
            Terminate bit
            Queue pointer = NULL
        */
        uhci_frame_list[i] = 0x00000001;
    }


    kprintf("[UHCI] Frame list allocated\n");
}



/*
    Setup frame scheduler
*/
void usb_uhci_frame_init(void)
{
    if (!uhci_frame_list)
    {
        kprintf("[UHCI] No frame list\n");
        return;
    }


    uint64_t addr =
        (uint64_t)uhci_frame_list;


    /*
        UHCI needs physical address
    */

    outl(
        uhci_io_base + UHCI_FLBASEADD,
        (uint32_t)addr
    );


    outw(
        uhci_io_base + UHCI_FRNUM,
        0
    );


    kprintf("[UHCI] Frame scheduler initialized\n");
}



/*
    Reset USB port
*/
void usb_uhci_port_reset(uint16_t port)
{

    uint16_t status =
        inw(port);


    if (!(status & UHCI_PORT_CONN))
    {
        kprintf("[UHCI] No device\n");
        return;
    }


    kprintf("[UHCI] Reset port\n");


    outw(
        port,
        status | UHCI_PORT_RESET
    );


    for(volatile int i = 0; i < 50000; i++)
        __asm__ volatile("nop");


    outw(
        port,
        status & ~UHCI_PORT_RESET
    );


    for(volatile int i = 0; i < 50000; i++)
        __asm__ volatile("nop");


    status = inw(port);


    if(status & UHCI_PORT_ENABLE)
    {
        kprintf("[UHCI] Port enabled\n");
    }
    else
    {
        kprintf("[UHCI] Port failed\n");
    }
}



/*
    Scan UHCI ports
*/
void usb_uhci_scan_ports(void)
{

    kprintf("[UHCI] Scanning ports\n");


    for(int i = 0; i < 2; i++)
    {

        uint16_t port =
            uhci_io_base +
            UHCI_PORTSC1 +
            (i * 2);


        uint16_t status =
            inw(port);


        kprintf("Port ");
        kprint_u64(i+1);


        kprintf(": ");
        kprint_u64(status);


        kprintf("\n");


        if(status & UHCI_PORT_CONN)
        {
            usb_uhci_port_reset(port);
        }
    }
}



/*
    Detect USB device
*/
void usb_uhci_device_detect(void)
{

    kprintf("[UHCI] Device detection\n");


    for(int i = 0; i < 2; i++)
    {

        uint16_t port =
            uhci_io_base +
            UHCI_PORTSC1 +
            (i * 2);


        uint16_t status =
            inw(port);


        if(status & UHCI_PORT_CONN)
        {
            kprintf("[USB] Device on port ");
            kprint_u64(i+1);
            kprintf("\n");
        }

    }
}

/*
    Initialize UHCI

    QEMU PIIX3:
    bus 0
    dev 1
    fun 2
*/
void usb_uhci_init(void)
{
    kprintf("[UHCI] Initializing...\n");


    uhci_io_base =
        usb_uhci_get_io_base(
            0,
            1,
            2
        );


    if(!uhci_io_base)
    {
        kprintf("[UHCI] Not found\n");
        return;
    }


    usb_uhci_reset();

    usb_uhci_alloc_frame_list();

    usb_uhci_frame_init();

    usb_uhci_start();

    usb_uhci_scan_ports();

    usb_uhci_device_detect();

    usb_uhci_dump();
}

// SECTION 2 END



typedef struct
{
    uint8_t caplength;

    uint32_t hcsparams1;
    uint32_t hcsparams2;
    uint32_t hcsparams3;

    uint32_t hccparams1;

} XHCI_CAPS;


XHCI_CAPS xhci_caps;

uint64_t xhci_mmio = 0;


/*
    Read xHCI Capability Registers
*/
void xhci_read_capabilities(void)
{
    if (!xhci_mmio)
    {
        kprintf("[xHCI] No MMIO base\n");
        return;
    }


    volatile uint8_t *base =
        (volatile uint8_t *)xhci_mmio;


    kprintf("========== xHCI Capabilities ==========\n");

    kprintf("MMIO: 0x");
    kprint_u64(xhci_mmio);


    xhci_caps.caplength =
        base[0x00];


    xhci_caps.hcsparams1 =
        *(volatile uint32_t *)(base + 0x04);


    xhci_caps.hcsparams2 =
        *(volatile uint32_t *)(base + 0x08);


    xhci_caps.hcsparams3 =
        *(volatile uint32_t *)(base + 0x0C);


    xhci_caps.hccparams1 =
        *(volatile uint32_t *)(base + 0x10);



    kprintf("\nCAPLENGTH: ");
    kprint_u64(xhci_caps.caplength);


    kprintf("\nHCSPARAMS1: ");
    kprint_u64(xhci_caps.hcsparams1);


    kprintf("\nHCSPARAMS2: ");
    kprint_u64(xhci_caps.hcsparams2);


    kprintf("\nHCSPARAMS3: ");
    kprint_u64(xhci_caps.hcsparams3);


    kprintf("\nHCCPARAMS1: ");
    kprint_u64(xhci_caps.hccparams1);


    kprintf("\n=======================================\n");
}

void xhci_init(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    uint64_t bar =
        pci_get_memory_base(
            bus,
            dev,
            fun
        );

    kprintf("BAR0: ");
    kprint_u64(xhci_mmio);
    kprintf("\n");

    xhci_mmio = bar;


    kprintf("[xHCI] Controller found\n");


    xhci_read_capabilities();
}

// SECTION 2 END
