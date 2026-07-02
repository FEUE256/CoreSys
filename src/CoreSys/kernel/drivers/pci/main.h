#pragma once

#include <drivers/serial/main.h>
#include <drivers/page/main.h>
#include <drivers/task/main.h>
#include <globe.h>

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
