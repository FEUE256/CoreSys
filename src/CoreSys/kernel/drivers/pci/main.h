#pragma once

#include <drivers/serial/main.h>
#include <drivers/page/main.h>
#include <drivers/task/main.h>
#include <globe.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_config_address(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun,
    uint16_t offset
)
{
    return
        (1 << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)dev << 11) |
        ((uint32_t)fun << 8) |
        (offset & 0xFC);
}


uint32_t pci_read32(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun,
    uint16_t offset
)
{
    uint32_t address =
        (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)dev << 11) |
        ((uint32_t)fun << 8) |
        (offset & 0xFC);


    outl(
        0xCF8,
        address
    );


    return inl(0xCFC);
}



/*
    Read 16 bit PCI register
*/

uint16_t pci_read16(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun,
    uint16_t offset
)
{
    outl(
        PCI_CONFIG_ADDRESS,
        pci_config_address(
            bus,
            dev,
            fun,
            offset
        )
    );


    return
        inw(
            PCI_CONFIG_DATA +
            (offset & 2)
        );
}


/*
    PCI Memory Base
*/

uint64_t pci_get_memory_base(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    uint16_t value =
        pci_read16(
            bus,
            dev,
            fun,
            0x20
        );


    return
        ((uint64_t)(value & 0xFFF0))
        << 16;
}

/*
    PCI Memory Limit
*/

uint64_t pci_get_memory_limit(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    uint16_t value =
        pci_read16(
            bus,
            dev,
            fun,
            0x22
        );


    return
        (((uint64_t)(value & 0xFFF0))
        << 16)
        | 0xFFFFF;
}

int pci_get_pcie_link(uint8_t bus,
                      uint8_t dev,
                      uint8_t fn,
                      uint8_t* speed,
                      uint8_t* width);

/*
    PCI CONFIG READ
*/

uint32_t pci_read(uint8_t bus,
                  uint8_t device,
                  uint8_t func,
                  uint8_t offset)
{
    uint32_t address =
        0x80000000 |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)func << 8) |
        (offset & 0xFC);

    outl(0xCF8, address);
    return inl(0xCFC);
}

uint32_t pci_read_config32(uint8_t bus,
                           uint8_t slot,
                           uint8_t func,
                           uint8_t offset)
{
    uint32_t address;

    address =
        (1U << 31) |
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);

    return inl(PCI_CONFIG_DATA);
}

uint32_t ihb_pci_read32(uint8_t bus, uint8_t slot,
                               uint8_t func, uint8_t offset)
{
    uint32_t address =
        (1U << 31) |
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}


/*
    Read 8 bit PCI register
*/

uint8_t pci_read8(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun,
    uint16_t offset
)
{
    outl(
        PCI_CONFIG_ADDRESS,
        pci_config_address(
            bus,
            dev,
            fun,
            offset
        )
    );


    return
        inb(
            PCI_CONFIG_DATA +
            (offset & 3)
        );
}

/*
    PCI HELPERS
*/

uint16_t pci_vendor(uint8_t bus,
                    uint8_t dev,
                    uint8_t fn)
{
    uint32_t v = pci_read(bus, dev, fn, 0);
    return (uint16_t)(v & 0xFFFF);
}

uint16_t pci_device(uint8_t bus,
                    uint8_t dev,
                    uint8_t fn)
{
    uint32_t v = pci_read(bus, dev, fn, 0);
    return (uint16_t)((v >> 16) & 0xFFFF);
}

/*
    PCI SCANNER
*/

void pci_write(uint8_t bus,
               uint8_t device,
               uint8_t func,
               uint8_t offset,
               uint32_t value)
{
    uint32_t address =
        0x80000000 |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)func << 8) |
        (offset & 0xFC);

    outl(0xCF8, address);
    outl(0xCFC, value);
}

void pci_scan(void)
{
    for (uint16_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t dev = 0; dev < 32; dev++)
        {
            for (uint8_t fn = 0; fn < 8; fn++)
            {
                uint32_t v = pci_read(bus, dev, fn, 0);

                uint16_t vendor = (uint16_t)(v & 0xFFFF);

                if (vendor == 0xFFFF)
                {
                    if (fn == 0)
                        break;
                    continue;
                }

                uint16_t device = (uint16_t)((v >> 16) & 0xFFFF);

                uint32_t class_reg =
                    pci_read(bus, dev, fn, 8);

                uint8_t class    = (class_reg >> 24) & 0xFF;
                uint8_t subclass = (class_reg >> 16) & 0xFF;
                uint8_t progif   = (class_reg >> 8)  & 0xFF;

                uint8_t header_type =
                    (pci_read(bus, dev, fn, 0x0C) >> 16) & 0xFF;

                kprintf(
                    "PCI %u:%u:%u V=%x D=%x C=%x S=%x P=%x\n",
                    bus,
                    dev,
                    fn,
                    vendor,
                    device,
                    class,
                    subclass,
                    progif
                );

                if (class == 0x01 && subclass == 0x08)
                {
                    uint32_t cmd = pci_read(bus, dev, fn, 0x04);
                    cmd |= (1 << 1); // Memory space
                    cmd |= (1 << 2); // Bus master
                    pci_write(bus, dev, fn, 0x04, cmd);

                    kprintf("NVMe PCI DMA enabled\n");
                }

                uint8_t speed = 0;
                uint8_t width = 0;

                if (pci_get_pcie_link(bus, dev, fn, &speed, &width) == 0)
                {
                    kprintf("PCIe Gen=%u width=x%u\n\n",
                            speed, width);
                }
                else
                {
                    kprintf("(no PCIe link info)\n\n");
                }

                /*
                    OPTIONAL: detect multifunction device
                */
                if (fn == 0 && !(header_type & 0x80))
                {
                    break;
                }
            }
        }
    }
}

int pci_get_pcie_link(uint8_t bus, uint8_t dev, uint8_t fn,
                      uint8_t* speed, uint8_t* width)
{
    uint8_t cap = pci_read(bus, dev, fn, 0x34) & 0xFF;

    while (cap)
    {
        uint32_t hdr = pci_read(bus, dev, fn, cap);

        if ((hdr & 0xFF) == 0x10)
        {
            uint32_t link = pci_read(bus, dev, fn, cap + 0x10);

            uint8_t raw_speed = link & 0xF;
            uint8_t raw_width = (link >> 4) & 0x3F;

            *width = raw_width;

            // translate PCIe encoding → Gen number
            switch (raw_speed)
            {
                case 1: *speed = 1; break; // Gen1
                case 2: *speed = 2; break; // Gen2
                case 3: *speed = 3; break; // Gen3
                case 4: *speed = 4; break; // Gen4
                default: *speed = 0; return -2;
            }

            return 0;
        }

        cap = (hdr >> 8) & 0xFF;
    }

    return -1;
}

void pcie_print_info() {
    uint8_t speed = 0;
    uint8_t width = 0;

    if (pci_get_pcie_link(0, 31, 2, &speed, &width) == 0)
    {
        kprintf("PCIe link speed=%u width=x%u\n", speed, width);
    }
    else
    {
        kprintf("No PCIe capability found\n");
    }
}

static inline uint32_t pci_get_bar5(uint8_t bus,
                                    uint8_t dev,
                                    uint8_t fn)
{
    return pci_read(bus, dev, fn, 0x24);
}

#define PCI_CAP_PTR 0x34

uint8_t pci_find_capability(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t cap_id)
{
    uint16_t status = (pci_read(bus,dev,fn,0x04)>>16)&0xFFFF;
    if (!(status & 0x10))
        return 0; // no capabilities list

    uint8_t ptr = pci_read(bus, dev, fn, PCI_CAP_PTR) & 0xFF;

    while (ptr)
    {
        uint32_t cap = pci_read(bus, dev, fn, ptr);
        uint8_t id = cap & 0xFF;

        if (id == cap_id)
            return ptr;

        ptr = (cap >> 8) & 0xFF;
    }

    return 0;
}

void pci_find_igpu(void)
{
    for (uint16_t bus = 0; bus < 256; bus++)
    for (uint8_t dev = 0; dev < 32; dev++)
    for (uint8_t fn = 0; fn < 8; fn++)
    {
        uint32_t v = pci_read(bus, dev, fn, 8);

        uint8_t class = (v >> 24) & 0xFF;

        if (class == 0x03) // Display controller
        {
            kprintf("GPU found at %u:%u:%u\n", bus, dev, fn);
        }
    }
}

void pci_igpu_info() {
    for (uint16_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t dev = 0; dev < 32; dev++)
        {
            for (uint8_t fn = 0; fn < 8; fn++)
            {
                uint32_t class_reg = pci_read(bus, dev, fn, 0x08);

                uint8_t class    = (class_reg >> 24) & 0xFF;
                uint8_t subclass = (class_reg >> 16) & 0xFF;
                uint8_t progif   = (class_reg >> 8)  & 0xFF;

                uint32_t bar0 = pci_read(bus, dev, fn, 0x10);
                uintptr_t mmio = (uintptr_t)(bar0 & ~0xF);

                uint32_t bar2 = pci_read(bus, dev, fn, 0x18);
                uintptr_t fb = (uintptr_t)(bar2 & ~0xF);

                if (class == 0x03) // Display controller
                {
                    kprintf(
                        "GPU bus=%x dev=%x fn=%x class=%x subclass=%x progif=%x mmio=%x fb=%x\n",
                        bus,
                        dev,
                        fn,
                        class,
                        subclass,
                        progif,
                        mmio,
                        fb
                    );
                }
            }
        }
    }
}

void pci_init(cs_task *self)
{
    (void)self;

    framebuffer = NULL;

    for (uint16_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t dev = 0; dev < 32; dev++)
        {
            for (uint8_t fn = 0; fn < 8; fn++)
            {
                uint32_t class_reg = pci_read(bus, dev, fn, 0x08);

                uint8_t class = (class_reg >> 24) & 0xFF;

                // ONLY GPU
                if (class == 0x03)
                {
                    uint32_t bar2 = pci_read(bus, dev, fn, 0x18);
                    uintptr_t fb = (uintptr_t)(bar2 & ~0xF);

                    framebuffer = (volatile uint32_t*)fb;

                    return;
                }
            }
        }
    }
}
void pci_deinit(cs_task *self) {
    (void)self;

    framebuffer = NULL;
}

#define PCI_CONFIG_SIZE 256
#define PCI_EXT_CONFIG_SIZE 4096

#define PCI_COMMAND_OFFSET 0x04
#define PCI_STATUS_OFFSET  0x06
#define PCI_CAP_PTR_OFFSET 0x34

/*
 * Read one PCI configuration register
 */
uint32_t pci_read_config32(uint8_t bus,
                           uint8_t slot,
                           uint8_t func,
                           uint8_t offset);


/*
 * 1. Read all PCI configuration registers
 */
void pci_read_all_config(uint8_t bus,
                         uint8_t slot,
                         uint8_t func,
                         uint32_t *buffer)
{
    for (uint16_t offset = 0;
         offset < PCI_CONFIG_SIZE;
         offset += 4)
    {
        buffer[offset / 4] =
            pci_read_config32(bus, slot, func, offset);
    }
}


/*
 * 2. Dump configuration space
 */
void pci_dump_config(uint8_t bus,
                     uint8_t slot,
                     uint8_t func)
{
    kprintf("PCI Configuration Space %u:%u:%u\n",
            bus, slot, func);

    for (uint16_t offset = 0;
         offset < PCI_CONFIG_SIZE;
         offset += 4)
    {
        uint32_t value =
            pci_read_config32(bus, slot, func, offset);

        kprintf("0x");
        kprint_u64(offset);
        kprintf(": ");

        kprint_u64(value);
        kprintf("\n");
    }
}


/*
 * 3. Detect PCI capabilities
 */
void pci_detect_capabilities(uint8_t bus,
                             uint8_t slot,
                             uint8_t func)
{
    uint32_t status =
        pci_read_config32(bus, slot, func, 0x04);

    /*
     * Status register bit 4:
     * Capabilities List
     */
    if (!(status & (1 << 20)))
    {
        kprintf("No PCI capabilities\n");
        return;
    }


    uint32_t cap =
        pci_read_config32(bus,
                          slot,
                          func,
                          PCI_CAP_PTR_OFFSET);


    uint8_t ptr = cap & 0xFF;


    while (ptr)
    {
        uint32_t capability =
            pci_read_config32(bus,
                              slot,
                              func,
                              ptr);


        uint8_t id = capability & 0xFF;
        uint8_t next = (capability >> 8) & 0xFF;


        kprintf("Capability ID: ");
        kprint_u64(id);
        kprintf("\n");


        ptr = next;
    }
}


/*
 * 4. Detect PCI extended capabilities
 */
void pci_detect_extended_capabilities(uint8_t bus,
                                      uint8_t slot,
                                      uint8_t func)
{
    for (uint16_t offset = 0x100;
         offset < PCI_EXT_CONFIG_SIZE;
         offset += 4)
    {
        uint32_t cap =
            pci_read_config32(bus,
                              slot,
                              func,
                              offset);


        if (cap == 0)
            continue;


        uint16_t id = cap & 0xFFFF;


        kprintf("Extended Capability at 0x");
        kprint_u64(offset);

        kprintf(" ID: ");
        kprint_u64(id);

        kprintf("\n");
    }
}

/*
 * 5. Read PCI command register
 */
uint16_t pci_read_command(uint8_t bus,
                          uint8_t slot,
                          uint8_t func)
{
    uint32_t value =
        pci_read_config32(bus,
                          slot,
                          func,
                          PCI_COMMAND_OFFSET);


    return value & 0xFFFF;
}


/*
 * 6. Read PCI status register
 */
uint16_t pci_read_status(uint8_t bus,
                         uint8_t slot,
                         uint8_t func)
{
    uint32_t value =
        pci_read_config32(bus,
                          slot,
                          func,
                          PCI_STATUS_OFFSET);


    return (value >> 16) & 0xFFFF;
}

int pci_is_bridge(
    uint8_t bus,
    uint8_t dev,
    uint8_t fun
)
{
    uint8_t class =
        pci_read8(bus,dev,fun,0x0B);

    uint8_t subclass =
        pci_read8(bus,dev,fun,0x0A);


    if(class == 0x06 &&
       subclass == 0x04)
    {
        return 1;
    }


    return 0;
}
