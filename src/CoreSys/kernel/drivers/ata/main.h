#pragma once

#include <drivers/pci/main.h>
#include <drivers/serial/main.h>
#include <mod/types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>

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

int ata_wait_drq(uint16_t base)
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

void ata_io_delay(void)
{
    // Reading the alternate status register 4 times gives the
    // required ~400ns delay after a drive select or command write.
    for (int i = 0; i < 4; i++) {
        inb(0x3F6);
    }
}

bool ata_wait_ready(uint32_t lba)
{
    uint8_t status;
    uint32_t timeout = 100000;

    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    do {
        status = inb(0x1F7);
    } while ((status & 0x80) && --timeout);

    if (timeout == 0) {
        if (debug != 2) { kprintf("[ATA] TIMEOUT waiting for BSY clear, status=%02x\n", status); }
        return false;
    }

    if (status & 0x01) {
        kprintf("[ATA] ERR set, status=%02x\n", status);
        return false;
    }

    timeout = 100000;
    do {
        status = inb(0x1F7);
    } while (!(status & 0x08) && --timeout);

    if (timeout == 0) {
        if (debug != 2) { kprintf("[ATA] TIMEOUT waiting for DRQ, status=%02x, lba: ", status); }
        if (debug != 2) {
            kprint_u64(lba);
            serial_write("\n");
        }
        return false;
    }

    return true;
}

static volatile int ata_lock = 0;

static inline void ata_lock_acquire(void)
{
    while (__atomic_test_and_set(&ata_lock, __ATOMIC_ACQUIRE))
    {
        // optional: pause instruction
        __asm__ volatile ("pause");
    }
}

static inline void ata_lock_release(void)
{
    __atomic_clear(&ata_lock, __ATOMIC_RELEASE);
}

bool ata_read_sector_io(uint32_t lba, void *buffer)
{
    ata_lock_acquire();

    outb(0x1F6, 0xF0 | ((lba >> 24) & 0x0F));
    ata_io_delay();

    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);

    if (!ata_wait_ready(lba)) {
        ata_lock_release();
        return false;
    }

    insw(0x1F0, buffer, 256);

    ata_lock_release();
    delay_rough();
    return true;
}

#define SECTOR_SIZE 512

extern int kprintf(const char *fmt, ...);

void ata_dump_lba_io(uint32_t lba)
{
    uint8_t buf[SECTOR_SIZE];

    if (!ata_read_sector_io(lba, buf))
    {
        kprintf("[ATA] LBA %u read failed\n", lba);
        return;
    }

    kprintf("[ATA] Dump LBA %u\n", lba);

    for (int i = 0; i < SECTOR_SIZE; i += 16)
    {
        // Hex part
        kprintf("%04x  ", i);

        for (int j = 0; j < 16; j++)
        {
            kprintf("%02x ", buf[i + j]);
        }

        kprintf(" | ");

        // ASCII part
        for (int j = 0; j < 16; j++)
        {
            uint8_t c = buf[i + j];

            if (c >= 32 && c <= 126)
                kprintf("%c", c);
            else
                kprintf(".");
        }

        kprintf("\n");
    }

    kprintf("[END LBA %u]\n", lba);
}

bool ata_write_sector_io(uint32_t lba, const void *buffer)
{
    ata_lock_acquire();

    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    outb(0x1F6, 0xF0 | ((lba >> 24) & 0x0F));
    ata_io_delay();

    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);

    if (!ata_wait_ready(lba)) {
        ata_lock_release();
        return false;
    }

    outsw(0x1F0, buffer, 256);

    // Wait for the drive to finish committing this sector internally
    // before allowing the next command to be issued.
    uint32_t commit_timeout = 100000;
    uint8_t status;
    do {
        status = inb(0x1F7);
    } while ((status & ATA_SR_BSY) && --commit_timeout);

    if (commit_timeout == 0) {
        if (debug != 2) { kprintf("[ATA] TIMEOUT waiting for write commit, lba=%u, status=%02x\n", lba, status); }
        ata_lock_release();
        return false;
    }

    ata_lock_release();
    delay_rough();
    return true;
}

void ata_read_blocks(uint32_t lba, void *buffer, uint32_t count)
{
    uint8_t *buf = (uint8_t *)buffer;

    // kprintf("0 ");
    for (uint32_t i = 0; i < count; i++)
    {
        // kprintf("1 ");
        ata_read_sector_io(lba + i, buf + (i * SECTOR_SIZE));
    }
    // kprintf("2 ");
}

void ata_write_blocks(uint32_t lba, const void *buffer, uint32_t count)
{
    const uint8_t *buf = (const uint8_t *)buffer;

    // kprintf("00 ");
    
    for (uint32_t i = 0; i < count; i++)
    {
        // kprintf("01 ");
        ata_write_sector_io(lba + i, buf + (i * SECTOR_SIZE));
    }
    // kprintf("02 ");

}
