#pragma once

#include <drivers/pci/main.h>
#include <drivers/serial/main.h>
#include <stdint.h>
#include <stddef.h>

/* ATA I/O ports (Primary + Secondary bus) */
#define ATA_PRIMARY_BASE   0x1F0
#define ATA_SECONDARY_BASE 0x170

#define ATA_DATA(base)     ((base) + 0)
#define ATA_ERROR(base)    ((base) + 1)
#define ATA_SECCOUNT0(base) ((base) + 2)
#define ATA_LBA0(base)     ((base) + 3)
#define ATA_LBA1(base)     ((base) + 4)
#define ATA_LBA2(base)     ((base) + 5)
#define ATA_HDDEVSEL(base) ((base) + 6)
#define ATA_COMMAND(base)  ((base) + 7)
#define ATA_STATUS(base)   ((base) + 7)

/* Commands */
#define ATA_CMD_READ_PIO 0x20

/* Status flags */
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

static inline uint16_t ata_base(int bus)
{
    return (bus == 0) ? ATA_PRIMARY_BASE : ATA_SECONDARY_BASE;
}

static void ata_wait_ready(uint16_t base)
{
    for (uint32_t i = 0; i < 1000000; i++)
    {
        uint8_t s = inb(ATA_STATUS(base));
        if (!(s & ATA_SR_BSY))
            return;
    }
}

static int ata_wait_drq(uint16_t base)
{
    for (uint32_t i = 0; i < 1000000; i++)
    {
        uint8_t s = inb(ATA_STATUS(base));

        if (s & ATA_SR_DRQ)
            return 0;

        if (s & ATA_SR_ERR)
            return -1;
    }

    return -1;
}

/*
 * dev:
 * 0 = master
 * 1 = slave
 */
static int ata_read_sector(int bus, int dev, uint32_t lba, void* buffer)
{
    if (dev > 1)
        return -1;

    uint16_t base = ata_base(bus);
    uint16_t* buf = (uint16_t*)buffer;

    uint8_t drive_select =
        0xE0 |
        ((dev & 1) << 4) |
        ((lba >> 24) & 0x0F);

    outb(ATA_HDDEVSEL(base), drive_select);
    ata_wait_ready(base);

    outb(ATA_SECCOUNT0(base), 1);

    outb(ATA_LBA0(base), (uint8_t)(lba));
    outb(ATA_LBA1(base), (uint8_t)(lba >> 8));
    outb(ATA_LBA2(base), (uint8_t)(lba >> 16));

    outb(ATA_COMMAND(base), ATA_CMD_READ_PIO);

    if (ata_wait_drq(base) != 0)
        return -1;

    for (int i = 0; i < 256; i++)
        buf[i] = inw(ATA_DATA(base));

    return 0;
}

static void hexdump(const void* data, size_t size)
{
    const uint8_t* p = (const uint8_t*)data;

    for (size_t i = 0; i < size; i++)
    {
        if ((i & 15) == 0)
            kprintf("%04x: ", (unsigned)i);

        kprintf("%02x ", p[i]);

        if ((i & 15) == 15)
            kprintf("\n");
    }

    kprintf("\n");
}

/*
 * Correct ATA scan:
 * - 2 buses
 * - 2 devices per bus
 * TOTAL: 4 possible devices
 */
void hexdump_512_all(void)
{
    uint8_t sector[512];

    for (int bus = 0; bus < 32; bus++)
    {
        for (int dev = 0; dev < 4; dev++)
        {
            kprintf("\n=== ATA BUS %d DEV %d ===\n", bus, dev);

            if (ata_read_sector(bus, dev, 0, sector) == 0)
            {
                hexdump(sector, 512);
            }
            else
            {
                kprintf("No response\n");
            }
        }
    }
}