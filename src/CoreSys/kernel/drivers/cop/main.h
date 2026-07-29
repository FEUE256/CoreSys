// CoreSyStem Omega Pro FileSyStem (COPFS)
// Namespace FEUE CoreSys Kernel COP
//
// Single-file rewrite. This file replaces drivers/cop/spec.h,
// drivers/cop/internal.h and drivers/cop/main.h. Include this
// file wherever those three were included, and remove the old ones.
//
// ---------------------------------------------------------------
// ON-DISK LAYOUT (dynamic, computed at format time)
// ---------------------------------------------------------------
//   Block 0            : superblock
//   Block bitmap_start  .. +bitmap_blocks : free-block bitmap
//   Block inode_start   .. +inode_blocks  : inode table
//   Block data_start    .. total_blocks   : file/dir data blocks
//
// bitmap_blocks and inode_blocks are computed from total_blocks and
// total_inodes in cop_format(), NOT hardcoded. The previous version
// hardcoded data_start = 20 while 1024 inodes actually require 54
// blocks (209 bytes/inode, 19 inodes/block) -- the inode table and
// the "free" data area overlapped by 36 blocks, guaranteeing silent
// corruption. That class of bug is now structurally impossible: the
// layout is derived from the struct sizes below.
//
// Struct byte offsets (all __attribute__((packed))), current version:
//
//   cop_superblock_t (94 bytes total)
//     0  magic[6]
//     6  version        (u32)
//     10 block_size     (u32)
//     14 total_blocks   (u64)
//     22 free_blocks    (u64)
//     30 total_inodes   (u64)
//     38 free_inodes    (u64)
//     46 bitmap_start   (u64)
//     54 bitmap_blocks  (u64)
//     62 inode_start    (u64)
//     70 inode_blocks   (u64)
//     78 data_start     (u64)
//     86 root_inode     (u64)
//
//   cop_inode_t (209 bytes total)
//     0  type           (u8)
//     1  size           (u64)
//     9  created        (u64)
//     17 blocks[24]     (u64 each, 192 bytes)
//
//   cop_dirent_t (211 bytes total)
//     0  inode          (u64)
//     8  type           (u8)
//     9  name_length    (u16)
//     11 name[200]
//
// If you change any of these structs, update ata.run (the host-side
// tool) to match -- the _Static_assert block below will catch size
// drift at kernel build time, but the host tool is a separate binary
// and will not be warned automatically.
// ---------------------------------------------------------------

#pragma once

// Standard libs
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// Standard includes
#include <drivers/serial/main.h>
#include <drivers/ata/main.h>
#include <drivers/task/main.h>
#include <mod/types.h>
#include <mod/globe.h>
#include <drivers/time/main.h>
#include <kernel/mem.h>
#include <drivers/cfs/main.h>
#include <drivers/ret/main.h>
#include <drivers/sf/main.h>
#include <drivers/halt/main.h>
#include <drivers/tty/state.h>

// ATA block definitions (read/write)
void ata_write_blocks(uint32_t lba, const void *buffer, uint32_t count);
void ata_read_blocks(uint32_t lba, void *buffer, uint32_t count);

// ---------------------------------------------------------------
// General information
// ---------------------------------------------------------------
#define COP_NAME     "COP"
#define COPFS_NAME   "COPFS"
#define COP_MAGIC    "COPFS"

// Bumped: on-disk superblock layout changed (fields added/reordered).
// Old images will fail cop_mount() cleanly instead of being
// misinterpreted. Reformat with fsf after upgrading.
#define COP_VERSION  0x00010002u

// ---------------------------------------------------------------
// Block / sector information
// ---------------------------------------------------------------
#define COP_SECTOR_SIZE   512
#define COP_LBA_SIZE      512
#define COP_BLOCK_SIZE    4096
#define COP_TOTAL_LBAS    131072   // 64 MB at COP_SECTOR_SIZE sectors
#define COP_RESERVED_LBAS 1
#define COP_EXEC_FILE_MAX_BYTE 8388608

// ---------------------------------------------------------------
// Superblock
// ---------------------------------------------------------------
typedef struct __attribute__((packed))
{
    char     magic[6];
    uint32_t version;
    uint32_t block_size;

    uint64_t total_blocks;
    uint64_t free_blocks;

    uint64_t total_inodes;
    uint64_t free_inodes;

    uint64_t bitmap_start;
    uint64_t bitmap_blocks;

    uint64_t inode_start;
    uint64_t inode_blocks;

    uint64_t data_start;
    uint64_t root_inode;
} cop_superblock_t;

cop_superblock_t cop_g_sb;

// ---------------------------------------------------------------
// Inode
// ---------------------------------------------------------------
// type: 0 = free, 1 = file, 2 = directory
#define COP_DIRECT_BLOCKS 86

typedef struct __attribute__((packed))
{
    uint8_t  type;
    uint64_t size;
    uint64_t created;
    uint64_t blocks[COP_DIRECT_BLOCKS];
} cop_inode_t;

// ---------------------------------------------------------------
// Directory entry
// ---------------------------------------------------------------
#define COP_MAX_NAME_LEN 200

typedef struct __attribute__((packed))
{
    uint64_t inode;
    uint8_t  type;
    uint16_t name_length;
    char     name[COP_MAX_NAME_LEN];
} cop_dirent_t;

// ---------------------------------------------------------------
// Compile-time layout guards -- catches struct drift immediately.
// If these fire, ata.run's mirrored structs are now out of sync.
// ---------------------------------------------------------------
_Static_assert(sizeof(cop_superblock_t) == 94,
    "cop_superblock_t size changed - update ata.run to match");
_Static_assert(sizeof(cop_inode_t) == 705,
    "cop_inode_t size changed - update ata.run to match");
_Static_assert(sizeof(cop_dirent_t) == 211,
    "cop_dirent_t size changed - update ata.run to match");

// ---------------------------------------------------------------
// Limits
// ---------------------------------------------------------------
#define COP_MAX_DISK_SIZE_MB   64
#define COP_MAX_DIRS           65500
#define COP_MAX_PATH_LEN_IN_DIRS 32767 // Windows long-path parity
#define COP_MIN_DISK_SIZE_MB   1.44
#define COP_MIN_BLOCK_SIZE     512

// Real max file size given COP_DIRECT_BLOCKS direct blocks and no
// indirect blocks. The old COP_MAX_FILE_SIZE_MB (8) was aspirational
// and did not match what the inode structure could actually address
// (24 * 4096 = 98304 bytes, ~96 KB). This is the enforced limit now.
#define COP_MAX_FILE_SIZE ((uint64_t)COP_DIRECT_BLOCKS * COP_BLOCK_SIZE)

// Default inode count used by cop_format(). Table size is computed
// from this, not hardcoded as a block count.
#define COP_DEFAULT_INODE_COUNT 1024

// ---------------------------------------------------------------
// Global state
// ---------------------------------------------------------------
// bool cop_g_inited  = false; // Defined in globe.h
// bool cop_g_mounted = false;

// ---------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------
uint64_t cop_alloc_block(void);
bool     cop_free_block(uint64_t block);
bool     cop_write_superblock(void);
bool     cop_format(uint32_t total_lbas);
bool     cop_mount(void);
void     cop_unmount(void);
bool     cop_read_inode(uint64_t inode, cop_inode_t *node);
bool     cop_write_inode(uint64_t inode, const cop_inode_t *node);
uint64_t cop_alloc_inode(void);
static bool cop_free_inode(uint64_t inode_num);
bool     cop_split_path(const char *path, char *parent, char *name);
bool     cop_add_dir_entry(uint64_t parent, const char *name, uint64_t inode, uint8_t type);
bool     cop_remove_dir_entry(uint64_t parent, const char *name);
bool     cop_lookup(const char *path, uint64_t *inode);
bool     cop_delete_recursive(const char *path);
uint64_t cop_katoi(const char *buf);

// =================================================================
// Time
// =================================================================
uint64_t cop_get_time(void)
{
    uint8_t sec   = cmos_read(0x00);
    uint8_t min   = cmos_read(0x02);
    uint8_t hour  = cmos_read(0x04);

    uint8_t day   = cmos_read(0x07);
    uint8_t month = cmos_read(0x08);
    uint16_t year = cmos_read(0x09);

    uint8_t regB = cmos_read(0x0B);

    if (!(regB & 0x04))
    {
        sec   = bcd_to_bin(sec);
        min   = bcd_to_bin(min);
        hour  = bcd_to_bin(hour);
        day   = bcd_to_bin(day);
        month = bcd_to_bin(month);
        year  = bcd_to_bin(year);
    }

    year += 2000;

    uint64_t timestamp = 0;

    timestamp |= ((uint64_t)year   & 0xFFFF) << 26;
    timestamp |= ((uint64_t)month  & 0x0F)   << 22;
    timestamp |= ((uint64_t)day    & 0x1F)   << 17;
    timestamp |= ((uint64_t)hour   & 0x1F)   << 12;
    timestamp |= ((uint64_t)min    & 0x3F)   << 6;
    timestamp |= ((uint64_t)sec    & 0x3F);

    return timestamp;
}

// =================================================================
// Superblock I/O
// =================================================================
bool cop_write_superblock(void)
{
    uint8_t sector_buf[COP_SECTOR_SIZE];
    memset(sector_buf, 0, sizeof(sector_buf));
    memcpy(sector_buf, &cop_g_sb, sizeof(cop_g_sb));

    ata_write_blocks(0, sector_buf, 1);

    return true;
}

// =================================================================
// Block <-> LBA
// =================================================================
static uint32_t cop_block_to_lba(uint64_t block)
{
    return (uint32_t)(block * (COP_BLOCK_SIZE / COP_SECTOR_SIZE));
}

// =================================================================
// Free-block bitmap (supports bitmaps spanning multiple blocks)
// =================================================================
static volatile int cop_bitmap_lock = 0;

static inline void cop_bitmap_lock_acquire(void)
{
    while (__atomic_test_and_set(&cop_bitmap_lock, __ATOMIC_ACQUIRE))
        __asm__ volatile ("pause");
}

static inline void cop_bitmap_lock_release(void)
{
    __atomic_clear(&cop_bitmap_lock, __ATOMIC_RELEASE);
}

uint64_t cop_alloc_block(void)
{
    if (!cop_g_mounted)
        return UINT64_MAX;

    cop_bitmap_lock_acquire();

    uint64_t data_blocks = cop_g_sb.total_blocks - cop_g_sb.data_start;
    uint64_t bits_per_block = (uint64_t)COP_BLOCK_SIZE * 8;

    uint8_t block_buf[COP_BLOCK_SIZE];

    for (uint64_t bb = 0; bb < cop_g_sb.bitmap_blocks; bb++)
    {
        ata_read_blocks(
            cop_block_to_lba(cop_g_sb.bitmap_start + bb),
            block_buf,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );

        uint64_t base_bit = bb * bits_per_block;

        for (uint64_t bit = 0; bit < bits_per_block; bit++)
        {
            uint64_t global_bit = base_bit + bit;

            if (global_bit >= data_blocks)
                break;

            uint64_t byte = bit / 8;
            uint8_t  mask = (uint8_t)(1u << (bit % 8));

            if (!(block_buf[byte] & mask))
            {
                block_buf[byte] |= mask;

                ata_write_blocks(
                    cop_block_to_lba(cop_g_sb.bitmap_start + bb),
                    block_buf,
                    COP_BLOCK_SIZE / COP_SECTOR_SIZE
                );

                cop_g_sb.free_blocks--;
                cop_write_superblock();

                cop_bitmap_lock_release();
                return cop_g_sb.data_start + global_bit;
            }
        }
    }

    cop_bitmap_lock_release();
    return UINT64_MAX;
}

bool cop_free_block(uint64_t block)
{
    if (!cop_g_mounted)
        return false;

    if (block < cop_g_sb.data_start || block >= cop_g_sb.total_blocks)
        return false;

    uint64_t global_bit = block - cop_g_sb.data_start;
    uint64_t bits_per_block = (uint64_t)COP_BLOCK_SIZE * 8;

    uint64_t bb  = global_bit / bits_per_block;
    uint64_t bit = global_bit % bits_per_block;

    if (bb >= cop_g_sb.bitmap_blocks)
        return false;

    uint8_t block_buf[COP_BLOCK_SIZE];

    ata_read_blocks(
        cop_block_to_lba(cop_g_sb.bitmap_start + bb),
        block_buf,
        COP_BLOCK_SIZE / COP_SECTOR_SIZE
    );

    uint64_t byte = bit / 8;
    uint8_t  mask = (uint8_t)(1u << (bit % 8));

    if (!(block_buf[byte] & mask))
        return false; // already free - guards against double free

    block_buf[byte] &= (uint8_t)~mask;

    ata_write_blocks(
        cop_block_to_lba(cop_g_sb.bitmap_start + bb),
        block_buf,
        COP_BLOCK_SIZE / COP_SECTOR_SIZE
    );

    cop_g_sb.free_blocks++;
    cop_write_superblock();

    return true;
}

// =================================================================
// Inode I/O
// =================================================================
bool cop_read_inode(uint64_t inode, cop_inode_t *node)
{
    if (!node || inode >= cop_g_sb.total_inodes)
        return false;

    uint64_t inodes_per_block = COP_BLOCK_SIZE / sizeof(cop_inode_t);

    uint64_t block = cop_g_sb.inode_start + (inode / inodes_per_block);
    uint32_t index  = (uint32_t)(inode % inodes_per_block);

    uint8_t buffer[COP_BLOCK_SIZE];

    ata_read_blocks(
        cop_block_to_lba(block),
        buffer,
        COP_BLOCK_SIZE / COP_SECTOR_SIZE
    );

    cop_inode_t *table = (cop_inode_t *)buffer;

    memcpy(node, &table[index], sizeof(cop_inode_t));

    return true;
}

bool cop_write_inode(uint64_t inode, const cop_inode_t *node)
{
    if (!node || inode >= cop_g_sb.total_inodes)
        return false;

    uint64_t inodes_per_block = COP_BLOCK_SIZE / sizeof(cop_inode_t);

    uint64_t block = cop_g_sb.inode_start + (inode / inodes_per_block);
    uint32_t index  = (uint32_t)(inode % inodes_per_block);

    uint8_t buffer[COP_BLOCK_SIZE];

    ata_read_blocks(
        cop_block_to_lba(block),
        buffer,
        COP_BLOCK_SIZE / COP_SECTOR_SIZE
    );

    cop_inode_t *table = (cop_inode_t *)buffer;

    memcpy(&table[index], node, sizeof(cop_inode_t));

    ata_write_blocks(
        cop_block_to_lba(block),
        buffer,
        COP_BLOCK_SIZE / COP_SECTOR_SIZE
    );

    return true;
}

uint64_t cop_alloc_inode(void)
{
    for (uint64_t i = 0; i < cop_g_sb.total_inodes; i++)
    {
        cop_inode_t inode;

        if (!cop_read_inode(i, &inode))
            continue;

        if (inode.type == 0)
        {
            memset(&inode, 0, sizeof(inode));
            inode.type = 1;

            if (!cop_write_inode(i, &inode))
                return UINT64_MAX;

            cop_g_sb.free_inodes--;
            cop_write_superblock();

            return i;
        }
    }

    return UINT64_MAX;
}

// Clears an inode back to free and persists the free-inode count.
// Used both by delete paths and by rollback on failed create/mkdir.
static bool cop_free_inode(uint64_t inode_num)
{
    cop_inode_t empty;
    memset(&empty, 0, sizeof(empty));

    if (!cop_write_inode(inode_num, &empty))
        return false;

    cop_g_sb.free_inodes++;
    cop_write_superblock();

    return true;
}

// =================================================================
// Format / mount / unmount
// =================================================================

// Zeroes the disk in large chunks via the heap instead of one
// 512-byte ata_write_blocks() call per sector. The old per-sector
// loop (131072 calls for a 64 MB disk) was hammering the emulated
// ATA controller faster than it could retire commands, producing
// BSY-stuck timeouts on scattered LBAs and leaving those sectors
// in an unknown state. Chunking cuts the call count by ~128x.
static void cop_clear_disk(uint32_t total_lbas)
{
    const uint32_t chunk_sectors = 128; // 64 KB per write call

    uint8_t *zero = (uint8_t *)kmalloc((size_t)chunk_sectors * COP_SECTOR_SIZE);

    if (!zero)
    {
        // Heap unavailable this early - fall back to slow single-sector path
        uint8_t single[COP_SECTOR_SIZE];
        memset(single, 0, sizeof(single));

        for (uint32_t lba = 0; lba < total_lbas; lba++)
            ata_write_blocks(lba, single, 1);

        return;
    }

    memset(zero, 0, (size_t)chunk_sectors * COP_SECTOR_SIZE);

    uint32_t lba = 0;

    while (lba < total_lbas)
    {
        uint32_t count = (total_lbas - lba) > chunk_sectors ?
                          chunk_sectors : (total_lbas - lba);

        ata_write_blocks(lba, zero, count);
        lba += count;
    }

    kfree(zero);
}

bool cop_format(uint32_t total_lbas)
{
    if (total_lbas == 0)
        return false;

    cop_clear_disk(total_lbas);

    cop_superblock_t sb;
    memset(&sb, 0, sizeof(sb));

    memcpy(sb.magic, COP_MAGIC, 5);
    sb.magic[5] = '\0';

    sb.version    = COP_VERSION;
    sb.block_size = COP_BLOCK_SIZE;

    sb.total_blocks = ((uint64_t)total_lbas * COP_SECTOR_SIZE) / COP_BLOCK_SIZE;

    // Layout derived from actual struct sizes, not guessed constants.
    uint64_t bits_per_block = (uint64_t)COP_BLOCK_SIZE * 8;

    sb.bitmap_start  = 1;
    sb.bitmap_blocks = (sb.total_blocks + bits_per_block - 1) / bits_per_block;

    if (sb.bitmap_blocks < 1)
        sb.bitmap_blocks = 1;

    sb.inode_start = sb.bitmap_start + sb.bitmap_blocks;

    sb.total_inodes = COP_DEFAULT_INODE_COUNT;

    uint64_t inodes_per_block = COP_BLOCK_SIZE / sizeof(cop_inode_t);
    sb.inode_blocks = (sb.total_inodes + inodes_per_block - 1) / inodes_per_block;

    sb.data_start = sb.inode_start + sb.inode_blocks;

    // Disk too small to hold the requested inode table plus at
    // least a handful of data blocks - refuse instead of silently
    // producing an overlapping layout.
    if (sb.data_start + 8 >= sb.total_blocks)
        return false;

    sb.free_blocks = sb.total_blocks - sb.data_start;
    sb.free_inodes = sb.total_inodes;
    sb.root_inode  = 0;

    memcpy(&cop_g_sb, &sb, sizeof(sb));

    if (!cop_write_superblock())
        return false;

    // Zero the bitmap region and inode table region explicitly
    // (cop_clear_disk already zeroed the whole disk, but this keeps
    // format() correct even if cop_clear_disk is ever skipped/changed).
    uint8_t zero_block[COP_BLOCK_SIZE];
    memset(zero_block, 0, sizeof(zero_block));

    for (uint64_t b = 0; b < sb.bitmap_blocks; b++)
    {
        ata_write_blocks(
            cop_block_to_lba(sb.bitmap_start + b),
            zero_block,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );
    }

    for (uint64_t b = 0; b < sb.inode_blocks; b++)
    {
        ata_write_blocks(
            cop_block_to_lba(sb.inode_start + b),
            zero_block,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );
    }

    cop_inode_t root;
    memset(&root, 0, sizeof(root));
    root.type    = 2;
    root.size    = 0;
    root.created = cop_get_time();

    if (!cop_write_inode(sb.root_inode, &root))
    {
        cop_g_mounted = false;
        return false;
    }

    cop_g_sb.free_inodes--;
    cop_write_superblock();

    return true;
}

bool cop_mount(void)
{
    uint64_t size;
    char *data = (char *)cfs_read(debug_cfg_file, &size);

    uint64_t debug = 0;

    if (data)
    {
        data[size] = '\0';
        debug = kstrtoull(data, NULL, 10);
    }

    if (cop_g_mounted) {
        if (debug != 2) { kprintf("cop_g_mounted = true already mounted, returning\n"); }
        return true;
    }

    uint8_t sector_buf[COP_SECTOR_SIZE];
    cop_superblock_t sb;

    ata_read_blocks(0, sector_buf, 1);
    memcpy(&sb, sector_buf, sizeof(sb));

    if (debug == 1) {
        kprintf("[COP] Magic: %s\n", sb.magic);
        kprintf("[COP] Version: %x\n", sb.version);
        kprintf("[COP] Block size: ");
        kprint_u64(sb.block_size);
        kprintf("\n");
    }

    if (memcmp(sb.magic, COP_MAGIC, 5) != 0)
    {
        if (debug != 2) { kprintf("[COP] Bad magic, formatting disk...\n"); }

        for (int i = 0; i < 3; i++)
        {
            if (!cop_format(COP_TOTAL_LBAS))
            {
                if (debug != 2) { kprintf("[COP] Format attempt %d failed.\n", i + 1); }
                continue;
            }

            if (debug != 2) { kprintf("[COP] Format successful, mounting...\n"); }

            if (cop_mount())
            {
                if (debug != 2) { kprintf("[COP] Format + mount successful.\n"); }
                return true;
                khlt();
            }

            if (debug != 2) { kprintf("[COP] Mount failed after format.\n"); }
        }

        k_sf("[COP] Failed to format and mount after 3 attempts. COPFS: Unable to initialize filesystem.");

        return false;
    }

    if (sb.version != COP_VERSION)
    {
        if (debug != 2) { kprintf("[COP] Version mismatch disk=%x kernel=%x\n",
            sb.version, COP_VERSION); }
        return false;
    }

    if (sb.version != COP_VERSION)
    {
        if (debug != 2) {
            kprintf("[COP] Version mismatch disk=%x kernel=%x\n",
                sb.version, COP_VERSION);
        }
        return false;
    }

    if (sb.block_size != COP_BLOCK_SIZE)
    {
        if (debug != 2) {
            kprintf("[COP] Block size mismatch disk=");
            kprint_u64(sb.block_size);
            kprintf(" kernel=%u\n", COP_BLOCK_SIZE);
        }
        return false;
    }

    if (sb.bitmap_blocks == 0 || sb.inode_blocks == 0)
    {
        if (debug != 2) { kprintf("[COP] Invalid bitmap/inode blocks\n"); }
        return false;
    }

    if (sb.inode_start <= sb.bitmap_start)
    {
        if (debug != 2) { kprintf("[COP] Invalid inode layout\n"); }
        return false;
    }

    if (sb.data_start <= sb.inode_start)
    {
        if (debug != 2) { kprintf("[COP] Invalid data start\n"); }
        return false;
    }

    if (sb.data_start >= sb.total_blocks)
    {
        if (debug != 2) { kprintf("[COP] Data outside disk\n"); }
        return false;
    }

    if (sb.root_inode >= sb.total_inodes)
    {
        if (debug != 2) { kprintf("[COP] Invalid root inode\n"); }
        return false;
    }

    memcpy(&cop_g_sb, &sb, sizeof(sb));
    cop_g_mounted = true;

    return true;
}

void cop_unmount(void)
{
    if (!cop_g_mounted)
        return;

    cop_g_mounted = false;
}

// =================================================================
// Path handling
// =================================================================
bool cop_split_path(const char *path, char *parent, char *name)
{
    if (!path || path[0] != '/')
        return false;

    uint64_t path_len = strlen(path);

    if (path_len == 0 || path_len >= COP_MAX_PATH_LEN_IN_DIRS)
        return false;

    char tmp[COP_MAX_PATH_LEN_IN_DIRS];
    strcpy(tmp, path);

    // Strip trailing slashes (except a lone "/")
    uint64_t tmp_len = strlen(tmp);

    while (tmp_len > 1 && tmp[tmp_len - 1] == '/')
    {
        tmp[tmp_len - 1] = '\0';
        tmp_len--;
    }

    const char *last = strrchr(tmp, '/');

    if (!last)
        return false;

    uint64_t name_len = strlen(last + 1);

    if (name_len == 0 || name_len >= COP_MAX_NAME_LEN)
        return false;

    strcpy(name, last + 1);

    if (last == tmp)
    {
        strcpy(parent, "/");
    }
    else
    {
        uint64_t len = (uint64_t)(last - tmp);
        memcpy(parent, tmp, len);
        parent[len] = '\0';
    }

    return true;
}

// =================================================================
// Directory entries
// =================================================================

// Adds a directory entry, allocating a new directory-table block
// on demand (and persisting the pointer into the parent inode)
// instead of assuming blocks[0] is already valid. Grows across up
// to COP_DIRECT_BLOCKS blocks as the directory fills up.
bool cop_add_dir_entry(uint64_t parent, const char *name, uint64_t inode, uint8_t type)
{
    if (!name || strlen(name) == 0 || strlen(name) >= COP_MAX_NAME_LEN)
        return false;

    cop_inode_t dir;

    if (!cop_read_inode(parent, &dir))
        return false;

    if (dir.type != 2)
        return false;

    cop_dirent_t entry;
    memset(&entry, 0, sizeof(entry));

    entry.inode       = inode;
    entry.type        = type;
    entry.name_length = (uint16_t)strlen(name);
    strcpy(entry.name, name);

    uint64_t entries_per_block = COP_BLOCK_SIZE / sizeof(cop_dirent_t);

    uint64_t entry_index = dir.size / sizeof(cop_dirent_t);
    uint64_t block_index = entry_index / entries_per_block;
    uint64_t block_offset = (entry_index % entries_per_block) * sizeof(cop_dirent_t);

    if (block_index >= COP_DIRECT_BLOCKS)
        return false;

    uint64_t block = dir.blocks[block_index];

    uint8_t buffer[COP_BLOCK_SIZE];

    if (block == 0)
    {
        block = cop_alloc_block();

        if (block == UINT64_MAX)
            return false;

        dir.blocks[block_index] = block;
        memset(buffer, 0, COP_BLOCK_SIZE);
    }
    else
    {
        ata_read_blocks(
            cop_block_to_lba(block),
            buffer,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );
    }

    memcpy(buffer + block_offset, &entry, sizeof(entry));

    ata_write_blocks(
        cop_block_to_lba(block),
        buffer,
        COP_BLOCK_SIZE / COP_SECTOR_SIZE
    );

    dir.size += sizeof(entry);

    return cop_write_inode(parent, &dir);
}

// Removes a directory entry by zeroing it in place (tombstone).
// cop_lookup() and directory iteration both already skip entries
// with inode == 0, so this does not require compaction.
bool cop_remove_dir_entry(uint64_t parent, const char *name)
{
    if (!name)
        return false;

    cop_inode_t dir;

    if (!cop_read_inode(parent, &dir))
        return false;

    if (dir.type != 2)
        return false;

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++)
    {
        if (dir.blocks[i] == 0)
            continue;

        uint8_t buffer[COP_BLOCK_SIZE];

        ata_read_blocks(
            cop_block_to_lba(dir.blocks[i]),
            buffer,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );

        uint64_t offset = 0;

        while (offset + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE)
        {
            cop_dirent_t *entry = (cop_dirent_t *)(buffer + offset);

            if (entry->inode != 0 && strcmp(entry->name, name) == 0)
            {
                memset(entry, 0, sizeof(cop_dirent_t));

                ata_write_blocks(
                    cop_block_to_lba(dir.blocks[i]),
                    buffer,
                    COP_BLOCK_SIZE / COP_SECTOR_SIZE
                );

                return true;
            }

            offset += sizeof(cop_dirent_t);
        }
    }

    return false;
}

// =================================================================
// Lookup
// =================================================================
bool cop_lookup(const char *path, uint64_t *inode)
{
    if (!path || !inode)
        return false;

    if (path[0] != '/')
        return false;

    if (strcmp(path, "/") == 0)
    {
        *inode = cop_g_sb.root_inode;
        return true;
    }

    uint64_t current_inode = cop_g_sb.root_inode;

    char component[COP_MAX_NAME_LEN];

    const char *p = path;

    if (*p == '/')
        p++;

    while (*p)
    {
        uint64_t len = 0;

        while (p[len] != '/' && p[len] != '\0')
        {
            len++;

            if (len >= COP_MAX_NAME_LEN)
                return false;
        }

        memcpy(component, p, len);
        component[len] = '\0';

        cop_inode_t dir;

        if (!cop_read_inode(current_inode, &dir))
            return false;

        if (dir.type != 2)
            return false;

        bool found = false;

        for (int i = 0; i < COP_DIRECT_BLOCKS; i++)
        {
            if (dir.blocks[i] == 0)
                continue;

            uint8_t buffer[COP_BLOCK_SIZE];

            ata_read_blocks(
                cop_block_to_lba(dir.blocks[i]),
                buffer,
                COP_BLOCK_SIZE / COP_SECTOR_SIZE
            );

            uint64_t offset = 0;

            while (offset + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE)
            {
                cop_dirent_t *entry = (cop_dirent_t *)(buffer + offset);

                if (entry->inode != 0 && strcmp(entry->name, component) == 0)
                {
                    current_inode = entry->inode;
                    found = true;
                    break;
                }

                offset += sizeof(cop_dirent_t);
            }

            if (found)
                break;
        }

        if (!found)
            return false;

        p += len;

        if (*p == '/')
            p++;
    }

    *inode = current_inode;

    return true;
}

// =================================================================
// Recursive delete
// =================================================================
bool cop_delete_recursive(const char *path)
{
    if (!path || !cop_g_mounted)
        return false;

    uint64_t inode_num;

    if (!cop_lookup(path, &inode_num))
        return false;

    if (inode_num == cop_g_sb.root_inode)
        return false;

    cop_inode_t inode;

    if (!cop_read_inode(inode_num, &inode))
        return false;

    if (inode.type == 2)
    {
        for (int i = 0; i < COP_DIRECT_BLOCKS; i++)
        {
            if (inode.blocks[i] == 0)
                continue;

            uint8_t buffer[COP_BLOCK_SIZE];

            ata_read_blocks(
                cop_block_to_lba(inode.blocks[i]),
                buffer,
                COP_BLOCK_SIZE / COP_SECTOR_SIZE
            );

            uint64_t offset = 0;

            while (offset + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE)
            {
                cop_dirent_t *entry = (cop_dirent_t *)(buffer + offset);

                if (entry->inode != 0)
                {
                    char child_path[COP_MAX_PATH_LEN_IN_DIRS];

                    if (strcmp(path, "/") == 0)
                        snprintf(child_path, sizeof(child_path), "/%s", entry->name);
                    else
                        snprintf(child_path, sizeof(child_path), "%s/%s", path, entry->name);

                    cop_delete_recursive(child_path);
                }

                offset += sizeof(cop_dirent_t);
            }
        }
    }

    char parent_path[COP_MAX_PATH_LEN_IN_DIRS];
    char name[COP_MAX_NAME_LEN];

    if (!cop_split_path(path, parent_path, name))
        return false;

    uint64_t parent_inode;

    if (cop_lookup(parent_path, &parent_inode))
        cop_remove_dir_entry(parent_inode, name);

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++)
    {
        if (inode.blocks[i] != 0)
            cop_free_block(inode.blocks[i]);
    }

    return cop_free_inode(inode_num);
}

// =================================================================
// Public API
// =================================================================

// cop_init(NULL) or cop_init(&some_task)
void cop_init(cs_task *self)
{
    (void)self;

    uint64_t size;
    char *data = (char *)cfs_read(debug_cfg_file, &size);

    uint64_t debug = 0;

    if (data)
    {
        data[size] = '\0';
        debug = kstrtoull(data, NULL, 10);
    }

    if (cop_g_inited) {
        if (debug != 2) { kprintf("Cop_g_inited = true returning\n"); }
        return;
    }

    bool mount = cop_mount();

    if (mount == true)
    {
        cop_g_inited = true;
        return;
    }

    k_sf("[COPFS] Failed to mount COPFS, run fsf in the cmd to format, Cant mount COPFS");
}

// cop_deinit(NULL) or cop_deinit(&some_task)
void cop_deinit(cs_task *self)
{
    (void)self;

    if (!cop_g_inited)
        return;

    cop_unmount();
    cop_g_inited = false;
}

bool cop_mkdir(const char *path)
{
    if (!path || !cop_g_mounted)
        return false;

    char parent[COP_MAX_PATH_LEN_IN_DIRS];
    char name[COP_MAX_NAME_LEN];

    if (!cop_split_path(path, parent, name))
        return false;

    if (strlen(name) == 0)
        return false;

    uint64_t exists_inode;

    if (cop_lookup(path, &exists_inode))
        return false;

    uint64_t parent_inode;

    if (!cop_lookup(parent, &parent_inode))
        return false;

    cop_inode_t parent_node;

    if (!cop_read_inode(parent_inode, &parent_node))
        return false;

    if (parent_node.type != 2)
        return false;

    uint64_t new_inode = cop_alloc_inode();

    if (new_inode == UINT64_MAX)
        return false;

    cop_inode_t dir;
    memset(&dir, 0, sizeof(dir));

    dir.type    = 2;
    dir.size    = 0;
    dir.created = cop_get_time();

    if (!cop_write_inode(new_inode, &dir))
    {
        cop_free_inode(new_inode);
        return false;
    }

    if (!cop_add_dir_entry(parent_inode, name, new_inode, 2))
    {
        cop_free_inode(new_inode); // avoid an orphaned inode
        return false;
    }

    return true;
}

bool cop_create(const char *path)
{
    if (!path || !cop_g_mounted)
        return false;

    char parent[COP_MAX_PATH_LEN_IN_DIRS];
    char name[COP_MAX_NAME_LEN];

    if (!cop_split_path(path, parent, name))
        return false;

    if (strlen(name) == 0)
        return false;

    uint64_t existing_inode;

    if (cop_lookup(path, &existing_inode))
        return false;

    uint64_t parent_inode;

    if (!cop_lookup(parent, &parent_inode))
        return false;

    cop_inode_t parent_node;

    if (!cop_read_inode(parent_inode, &parent_node))
        return false;

    if (parent_node.type != 2)
        return false;

    uint64_t new_inode = cop_alloc_inode();

    if (new_inode == UINT64_MAX)
        return false;

    cop_inode_t file;
    memset(&file, 0, sizeof(file));

    file.type    = 1;
    file.size    = 0;
    file.created = cop_get_time();

    if (!cop_write_inode(new_inode, &file))
    {
        cop_free_inode(new_inode);
        return false;
    }

    if (!cop_add_dir_entry(parent_inode, name, new_inode, 1))
    {
        cop_free_inode(new_inode);
        return false;
    }

    return true;
}

// Overwrites file contents from offset 0. Frees any blocks left
// over if the new content is shorter than what was there before.
bool cop_write(const char *path, const void *buffer, uint64_t size)
{
    if (!path || !buffer || !cop_g_mounted)
        return false;

    if (size > COP_MAX_FILE_SIZE)
        return false;

    uint64_t inode_num;

    if (!cop_lookup(path, &inode_num))
        return false;

    cop_inode_t inode;

    if (!cop_read_inode(inode_num, &inode))
        return false;

    if (inode.type != 1)
        return false;

    uint64_t old_blocks_used  = (inode.size + COP_BLOCK_SIZE - 1) / COP_BLOCK_SIZE;
    uint64_t new_blocks_needed = (size + COP_BLOCK_SIZE - 1) / COP_BLOCK_SIZE;

    uint64_t remaining = size;
    uint64_t offset = 0;
    uint64_t block_index = 0;

    while (remaining > 0)
    {
        uint64_t block = inode.blocks[block_index];

        if (block == 0)
        {
            block = cop_alloc_block();

            if (block == UINT64_MAX)
                return false;

            inode.blocks[block_index] = block;
        }

        uint8_t block_buffer[COP_BLOCK_SIZE];
        memset(block_buffer, 0, COP_BLOCK_SIZE);

        uint64_t write_size = remaining > COP_BLOCK_SIZE ? COP_BLOCK_SIZE : remaining;

        memcpy(block_buffer, (const uint8_t *)buffer + offset, write_size);

        ata_write_blocks(
            cop_block_to_lba(block),
            block_buffer,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );

        remaining -= write_size;
        offset    += write_size;
        block_index++;
    }

    // Shrink: free any blocks the file no longer needs.
    for (uint64_t i = new_blocks_needed; i < old_blocks_used && i < COP_DIRECT_BLOCKS; i++)
    {
        if (inode.blocks[i] != 0)
        {
            cop_free_block(inode.blocks[i]);
            inode.blocks[i] = 0;
        }
    }

    inode.size = size;

    return cop_write_inode(inode_num, &inode);
}

// Reads up to `size` bytes. The output buffer is always zeroed
// first for the full requested size, so short/empty files never
// leave stack garbage in unfilled bytes.
bool cop_read(const char *path, void *buffer, uint64_t size)
{
    if (!path || !buffer || !cop_g_mounted)
        return false;

    memset(buffer, 0, size);

    uint64_t inode_num;

    if (!cop_lookup(path, &inode_num))
    {
        if (strncmp(path, "/bin", 4) == 0)
        {
            k_sf("Run make all qemu to fix the error\n");
        }

        if (!cop_create(path))
            return false;

        if (!cop_lookup(path, &inode_num))
            return false;
    }

    cop_inode_t inode;

    if (!cop_read_inode(inode_num, &inode))
        return false;

    if (inode.type != 1)
        return false;

    uint64_t to_read = size > inode.size ? inode.size : size;

    uint64_t remaining = to_read;
    uint64_t offset = 0;
    uint64_t block_index = 0;

    while (remaining > 0 && block_index < COP_DIRECT_BLOCKS)
    {
        uint64_t block = inode.blocks[block_index];

        if (block == 0)
            break;

        uint8_t block_buffer[COP_BLOCK_SIZE];

        ata_read_blocks(
            cop_block_to_lba(block),
            block_buffer,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );

        uint64_t read_size = remaining > COP_BLOCK_SIZE ? COP_BLOCK_SIZE : remaining;

        memcpy((uint8_t *)buffer + offset, block_buffer, read_size);

        remaining -= read_size;
        offset    += read_size;
        block_index++;
    }

    return true;
}

bool cop_delete(const char *path)
{
    if (!path || !cop_g_mounted)
        return false;

    uint64_t inode_num;

    if (!cop_lookup(path, &inode_num))
        return false;

    if (inode_num == cop_g_sb.root_inode)
        return false;

    cop_inode_t inode;

    if (!cop_read_inode(inode_num, &inode))
        return false;

    if (inode.type == 2)
        return cop_delete_recursive(path);

    char parent_path[COP_MAX_PATH_LEN_IN_DIRS];
    char name[COP_MAX_NAME_LEN];

    if (!cop_split_path(path, parent_path, name))
        return false;

    uint64_t parent_inode;

    if (!cop_lookup(parent_path, &parent_inode))
        return false;

    if (!cop_remove_dir_entry(parent_inode, name))
        return false;

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++)
    {
        if (inode.blocks[i] != 0)
            cop_free_block(inode.blocks[i]);
    }

    return cop_free_inode(inode_num);
}

bool cop_append(const char *path, const void *buffer, uint64_t size)
{
    if (!path || !buffer || !cop_g_mounted)
        return false;

    if (size == 0)
        return true;

    uint64_t inode_num;

    if (!cop_lookup(path, &inode_num))
        return false;

    cop_inode_t inode;

    if (!cop_read_inode(inode_num, &inode))
        return false;

    if (inode.type != 1)
        return false;

    uint64_t old_size = inode.size;
    uint64_t new_size  = old_size + size;

    if (new_size > COP_MAX_FILE_SIZE)
        return false;

    uint64_t offset = old_size;
    uint64_t remaining = size;

    while (remaining > 0)
    {
        uint64_t block_index  = offset / COP_BLOCK_SIZE;
        uint64_t block_offset = offset % COP_BLOCK_SIZE;

        if (block_index >= COP_DIRECT_BLOCKS)
            return false;

        uint64_t block = inode.blocks[block_index];

        if (block == 0)
        {
            block = cop_alloc_block();

            if (block == UINT64_MAX)
                return false;

            inode.blocks[block_index] = block;
        }

        uint8_t block_buffer[COP_BLOCK_SIZE];

        if (block_offset != 0 || remaining < COP_BLOCK_SIZE)
        {
            ata_read_blocks(
                cop_block_to_lba(block),
                block_buffer,
                COP_BLOCK_SIZE / COP_SECTOR_SIZE
            );
        }
        else
        {
            memset(block_buffer, 0, COP_BLOCK_SIZE);
        }

        uint64_t write_size = COP_BLOCK_SIZE - block_offset;

        if (write_size > remaining)
            write_size = remaining;

        memcpy(
            block_buffer + block_offset,
            (const uint8_t *)buffer + (size - remaining),
            write_size
        );

        ata_write_blocks(
            cop_block_to_lba(block),
            block_buffer,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );

        offset    += write_size;
        remaining -= write_size;
    }

    inode.size = new_size;

    return cop_write_inode(inode_num, &inode);
}

// What it does: executes the path .bin
uint64_t cop_exec_file(const char *path)
{
    void *buffer = kmalloc(COP_EXEC_FILE_MAX_BYTE); // COP_MAX_FILE_SIZE_MB into bytes

    if (!buffer)
        return 1;

    cop_read(path, buffer, COP_EXEC_FILE_MAX_BYTE);

    __asm__ volatile (
        "lea 1f(%%rip), %%rax\n"  // Address after the jmp
        "push %%rax\n"            // Push return address
        "jmp *%0\n"               // Jump to buffer
        "1:\n"
        :
        : "r"(buffer)
        : "rax", "memory"
    );

    uint64_t status = *(volatile uint64_t *)KUE;

    uint8_t debug = *(volatile uint8_t *)KDI;

    if (debug == 1) {
        kprintf("[TRACE] Util: %s, returned", path);
        kprintf("\n");
    }

    kfree(buffer);

    return status;
}

// =================================================================
// Boot count helper
// =================================================================
uint64_t cop_katoi(const char *buf)
{
    uint64_t value = 0;

    while (*buf >= '0' && *buf <= '9')
    {
        value = value * 10 + (uint64_t)(*buf - '0');
        buf++;
    }

    return value;
}

/*
    File structure:
    /
    +-- sys
        +-- kernel
        |    +-- kernel.cfg
        +-- system
            +-- debug.cfg
            +-- boot
                +-- bc.sctfi
    +-- task
        +--  [tid]
    +-- bin
        +-- it.bin
*/

bool cop_exists(const char *path)
{
    if (!path || !cop_g_mounted)
        return false;

    uint64_t inode;

    return cop_lookup(path, &inode);
}

void fs_init(cs_task *self)
{
    (void)self;

    uint64_t size;
    char *data = (char *)cfs_read(debug_cfg_file, &size);

    uint64_t debug = 0;

    if (data)
    {
        data[size] = '\0';
        debug = kstrtoull(data, NULL, 10);
    }

    if (!cop_exists("/sys"))
    {
        cop_mkdir("/sys");
        if (debug == 1) kprintf("[COPFS] Created /sys\n");
    }
    else if (debug == 1)
    {
        kprintf("[COPFS] Exists /sys\n");
    }


    if (!cop_exists("/sys/kernel"))
    {
        cop_mkdir("/sys/kernel");
        if (debug == 1) kprintf("[COPFS] Created /sys/kernel\n");
    }
    else if (debug == 1)
    {
        kprintf("[COPFS] Exists /sys/kernel\n");
    }


    if (!cop_exists("/sys/system"))
    {
        cop_mkdir("/sys/system");
        if (debug == 1) kprintf("[COPFS] Created /sys/system\n");
    }
    else if (debug == 1)
    {
        kprintf("[COPFS] Exists /sys/system\n");
    }


    if (!cop_exists("/sys/system/boot"))
    {
        cop_mkdir("/sys/system/boot");
        if (debug == 1) kprintf("[COPFS] Created /sys/system/boot\n");
    }
    else if (debug == 1)
    {
        kprintf("[COPFS] Exists /sys/system/boot\n");
    }


    if (!cop_exists("/sys/kernel/kernel.cfg"))
    {
        cop_create("/sys/kernel/kernel.cfg");
        if (debug == 1) kprintf("[COPFS] Created /sys/kernel/kernel.cfg\n");
    }
    else if (debug == 1)
    {
        kprintf("[COPFS] Exists /sys/kernel/kernel.cfg\n");
    }


    if (!cop_exists("/sys/system/debug.cfg"))
    {
        cop_create("/sys/system/debug.cfg");
        if (debug == 1) kprintf("[COPFS] Created /sys/system/debug.cfg\n");
    }
    else if (debug == 1)
    {
        kprintf("[COPFS] Exists /sys/system/debug.cfg\n");
    }


    if (!cop_exists("/sys/system/boot/bc.sctfi"))
    {
        cop_create("/sys/system/boot/bc.sctfi");
        if (debug == 1) kprintf("[COPFS] Created /sys/system/boot/bc.sctfi\n");
    }
    else if (debug == 1)
    {
        kprintf("[COPFS] Exists /sys/system/boot/bc.sctfi\n");
    }

    if (!cop_exists("/task"))
    {
        cop_mkdir("/task");
        if (debug == 1) kprintf("[COPFS] Created /task\n");
    } else if (cop_exists("/task"))
    {
        cop_create("/task/.cfsts");
        if (debug == 1) {
            kprintf("[COPFS] Exists /task\n");
        }
    }

    char kernel_cfg_buffer[4092];
    uint64_t kernel_cfg_size = 0;

    uint64_t *kernel_data = cfs_read(kernel_cfg_file, &kernel_cfg_size);

    if (kernel_data && kernel_cfg_size < sizeof(kernel_cfg_buffer))
    {
        memcpy(kernel_cfg_buffer, kernel_data, kernel_cfg_size);
        kernel_cfg_buffer[kernel_cfg_size] = '\0';

        cop_write("/sys/kernel/kernel.cfg", kernel_cfg_buffer, kernel_cfg_size);
    }

    char debug_buffer[256];
    uint64_t debug_size = 0;

    char *debug_data = (char *)cfs_read(debug_cfg_file, &debug_size);

    if (debug_data && debug_size < sizeof(debug_buffer))
    {
        memcpy(debug_buffer, debug_data, debug_size);
        debug_buffer[debug_size] = '\0';

        cop_write("/sys/system/debug.cfg", debug_buffer, debug_size);
    }

    char bc_buffer[512];
    cop_read("/sys/system/boot/bc.sctfi", bc_buffer, sizeof(bc_buffer));

    uint64_t bc = cop_katoi(bc_buffer);
    bc += 1;

    bc_to_buf(bc, bc_buffer, sizeof(bc_buffer));
    cop_write("/sys/system/boot/bc.sctfi", bc_buffer, sizeof(bc_buffer));
}

void fs_deinit(cs_task *self)
{
    (void)self;

    cop_delete("/task"); // Task TID cleanup

    // Sync FS when we implement that
}

int bc_print(char *buf) {
    cop_read("/sys/system/boot/bc.sctfi", buf, sizeof(buf));
    return 0;
}

int tree_seen(uint64_t inode)
{
    for (int i = 0; i < tree_visited_count; i++)
    {
        if (tree_visited[i] == inode)
            return 1;
    }

    if (tree_visited_count < TREE_MAX_VISITED)
    {
        tree_visited[tree_visited_count++] = inode;
    }

    return 0;
}

void tree_print(const char *path, int depth)
{
    if (depth >= 16)
        return;


    uint64_t inode_num;

    if (!cop_lookup(path, &inode_num))
        return;


    if (tree_seen(inode_num))
    {
        for (int i = 0; i < depth; i++)
            kprintf("|   ");

        kprintf("[LOOP]\r\n");
        return;
    }


    cop_inode_t inode;

    cop_read_inode(inode_num, &inode);


    if (inode.type != 2)
        return;



    uint64_t total_entries = inode.size / sizeof(cop_dirent_t);
    uint64_t entries_seen = 0;


    char names[64][COP_MAX_NAME_LEN];
    uint8_t types[64];

    int count = 0;



    for (int i = 0;
         i < COP_DIRECT_BLOCKS && entries_seen < total_entries;
         i++)
    {
        if (inode.blocks[i] == 0)
            continue;

        if (inode.blocks[i] < cop_g_sb.data_start)
            continue;

        if (inode.blocks[i] >= cop_g_sb.total_blocks)
            continue;



        uint8_t buf[COP_BLOCK_SIZE];


        ata_read_blocks(
            cop_block_to_lba(inode.blocks[i]),
            buf,
            COP_BLOCK_SIZE / COP_SECTOR_SIZE
        );



        uint64_t offset = 0;


        while (offset + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE &&
               entries_seen < total_entries &&
               count < 64)
        {

            cop_dirent_t *entry =
                (cop_dirent_t *)(buf + offset);



            if (entry->inode != 0 &&
                entry->name[0] != '\0' &&
                strcmp(entry->name, ".") != 0 &&
                strcmp(entry->name, "..") != 0)
            {
                strcpy(names[count], entry->name);
                types[count] = entry->type;
                count++;
            }


            entries_seen++;
            offset += sizeof(cop_dirent_t);
        }
    }

    for (int i = 0; i < count; i++)
    {
        int is_last = (i == count - 1);

        for (int d = 0; d < depth; d++)
            kprintf("|   ");

        kprintf(is_last ? "+-- " : "|-- ");
        kprintf("%s\r\n", names[i]);

        if (types[i] == 2)
        {
            char child[256];

            if (path[0] == '/' && path[1] == '\0')
            {
                snprintf(child, sizeof(child), "/%s", names[i]);
            }
            else
            {
                snprintf(child, sizeof(child), "%s/%s", path, names[i]);
            }

            tree_print(child, depth + 1);
        }
    }
}

// THE ONLY UP TO DATE FS TREE IS BEFORE cop_exists
