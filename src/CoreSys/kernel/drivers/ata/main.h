#pragma once

#include <drivers/pci/main.h>
#include <drivers/serial/main.h>
#include <stdint.h>
#include <stddef.h>

#define ATA_DATA       0x1F0
#define ATA_ERROR      0x1F1
#define ATA_SECCOUNT0  0x1F2
#define ATA_LBA0       0x1F3
#define ATA_LBA1       0x1F4
#define ATA_LBA2       0x1F5
#define ATA_HDDEVSEL   0x1F6
#define ATA_COMMAND    0x1F7
#define ATA_STATUS     0x1F7

#define ATA_CMD_READ_PIO 0x20
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08

static void ata_wait_ready(void)
{
    for (uint32_t i = 0; i < 1000000; i++)
    {
        uint8_t s = inb(ATA_STATUS);
        if (!(s & ATA_SR_BSY))
            return;
    }
}

static int ata_wait_drq(void)
{
    for (uint32_t i = 0; i < 1000000; i++)
    {
        uint8_t s = inb(ATA_STATUS);

        if (s & ATA_SR_DRQ)
            return 0;

        if (s & 0x01) // ERR
            return -1;
    }

    return -1;
}

static int ata_read_sector_device(int dev, uint32_t lba, void* buffer)
{
    uint16_t* buf = (uint16_t*)buffer;

    uint8_t drive = 0xE0 | ((dev & 1) << 4) | ((lba >> 24) & 0x0F);

    outb(ATA_HDDEVSEL, drive);
    ata_wait_ready();

    outb(ATA_SECCOUNT0, 1);
    outb(ATA_LBA0, (uint8_t)lba);
    outb(ATA_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_READ_PIO);

    if (ata_wait_drq() != 0)
        return -1;

    for (int i = 0; i < 256; i++)
        buf[i] = inw(ATA_DATA);

    return 0;
}

static void hexdump(const void* data, size_t size)
{
    const uint8_t* p = data;

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

void hexdump_512_all(void)
{
    uint8_t sector[512];

    for (int dev = 0; dev < 4; dev++)
    {
        kprintf("\n=== ATA DEVICE %d ===\n", dev);

        if (ata_read_sector_device(dev, 0, sector) == 0)
        {
            hexdump(sector, 512);
        }
        else
        {
            kprintf("Device %d not responding\n", dev);
        }
    }
}
