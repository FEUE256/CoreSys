// copsh.c - Interactive host-side shell for COPFS disk images.
// Companion to ata.run. Mirrors the on-disk structures in copfs.h
// exactly (94-byte superblock, 209-byte inode, 211-byte dirent) so
// it reads and writes the same layout the CoreSys kernel produces.
//
// Build:
//   gcc -O2 -Wall -Wextra -o copsh copsh.c
//
// Usage:
//   ./copsh <disk-image>
//
// Every command below accepts -h/--help. Commands that have a
// meaningful modifier also accept a matching -x/--long-name flag.
// Run "help" inside the shell for the full command reference.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

// -----------------------------------------------------------------
// On-disk structures (must match copfs.h byte-for-byte)
// -----------------------------------------------------------------
#define COP_MAGIC          "COPFS"
#define COP_VERSION        0x00010002u
#define COP_SECTOR_SIZE    512
#define COP_BLOCK_SIZE     4096
#define COP_DIRECT_BLOCKS  86
#define COP_MAX_NAME_LEN   200
#define COP_MAX_PATH       32767

#pragma pack(push, 1)
typedef struct {
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

typedef struct {
    uint8_t  type; // 0 = free, 1 = file, 2 = directory
    uint64_t size;
    uint64_t created;
    uint64_t blocks[COP_DIRECT_BLOCKS];
} cop_inode_t;

typedef struct {
    uint64_t inode;
    uint8_t  type;
    uint16_t name_length;
    char     name[COP_MAX_NAME_LEN];
} cop_dirent_t;
#pragma pack(pop)

_Static_assert(sizeof(cop_superblock_t) == 94,  "superblock layout drift - resync with copfs.h");
_Static_assert(sizeof(cop_inode_t)      == 705, "inode layout drift - resync with copfs.h");
_Static_assert(sizeof(cop_dirent_t)     == 211, "dirent layout drift - resync with copfs.h");

// -----------------------------------------------------------------
// Global state
// -----------------------------------------------------------------
static FILE             *g_disk = NULL;
static cop_superblock_t  g_sb;
static uint64_t          g_cwd_inode;
static char              g_cwd_path[COP_MAX_PATH] = "/";
static bool              g_dirty_prompt_shown = false;

// =================================================================
// Raw block / superblock I/O
// =================================================================
static uint32_t block_to_lba(uint64_t block) {
    return (uint32_t)(block * (COP_BLOCK_SIZE / COP_SECTOR_SIZE));
}

static bool disk_read_blocks(uint32_t lba, void *buf, uint32_t count) {
    if (fseeko(g_disk, (off_t)lba * COP_SECTOR_SIZE, SEEK_SET) != 0) return false;
    return fread(buf, COP_SECTOR_SIZE, count, g_disk) == count;
}

static bool disk_write_blocks(uint32_t lba, const void *buf, uint32_t count) {
    if (fseeko(g_disk, (off_t)lba * COP_SECTOR_SIZE, SEEK_SET) != 0) return false;
    if (fwrite(buf, COP_SECTOR_SIZE, count, g_disk) != count) return false;
    fflush(g_disk);
    return true;
}

static bool block_read(uint64_t block, void *buf) {
    return disk_read_blocks(block_to_lba(block), buf, COP_BLOCK_SIZE / COP_SECTOR_SIZE);
}

static bool block_write(uint64_t block, const void *buf) {
    return disk_write_blocks(block_to_lba(block), buf, COP_BLOCK_SIZE / COP_SECTOR_SIZE);
}

static bool sb_load(void) {
    uint8_t sector[COP_SECTOR_SIZE];
    if (!disk_read_blocks(0, sector, 1)) return false;
    memcpy(&g_sb, sector, sizeof(g_sb));
    if (memcmp(g_sb.magic, COP_MAGIC, 5) != 0) {
        fprintf(stderr, "copsh: bad magic - this doesn't look like a COPFS image\n");
        return false;
    }
    if (g_sb.version != COP_VERSION) {
        fprintf(stderr, "copsh: warning - version mismatch (disk=0x%08x tool=0x%08x)\n",
                g_sb.version, COP_VERSION);
    }
    return true;
}

static bool sb_save(void) {
    uint8_t sector[COP_SECTOR_SIZE];
    memset(sector, 0, sizeof(sector));
    memcpy(sector, &g_sb, sizeof(g_sb));
    return disk_write_blocks(0, sector, 1);
}

// =================================================================
// Inode I/O
// =================================================================
static bool inode_read(uint64_t idx, cop_inode_t *out) {
    if (idx >= g_sb.total_inodes) return false;
    uint64_t per_block = COP_BLOCK_SIZE / sizeof(cop_inode_t);
    uint64_t block = g_sb.inode_start + idx / per_block;
    uint32_t off = (uint32_t)(idx % per_block);
    uint8_t buf[COP_BLOCK_SIZE];
    if (!block_read(block, buf)) return false;
    memcpy(out, buf + (size_t)off * sizeof(cop_inode_t), sizeof(cop_inode_t));
    return true;
}

static bool inode_write(uint64_t idx, const cop_inode_t *in) {
    if (idx >= g_sb.total_inodes) return false;
    uint64_t per_block = COP_BLOCK_SIZE / sizeof(cop_inode_t);
    uint64_t block = g_sb.inode_start + idx / per_block;
    uint32_t off = (uint32_t)(idx % per_block);
    uint8_t buf[COP_BLOCK_SIZE];
    if (!block_read(block, buf)) return false;
    memcpy(buf + (size_t)off * sizeof(cop_inode_t), in, sizeof(cop_inode_t));
    return block_write(block, buf);
}

static uint64_t inode_alloc(uint8_t type) {
    for (uint64_t i = 0; i < g_sb.total_inodes; i++) {
        cop_inode_t nd;
        if (!inode_read(i, &nd)) continue;
        if (nd.type == 0) {
            memset(&nd, 0, sizeof(nd));
            nd.type = type;
            if (!inode_write(i, &nd)) return UINT64_MAX;
            g_sb.free_inodes--;
            sb_save();
            return i;
        }
    }
    return UINT64_MAX;
}

static bool inode_free(uint64_t idx) {
    cop_inode_t empty;
    memset(&empty, 0, sizeof(empty));
    if (!inode_write(idx, &empty)) return false;
    g_sb.free_inodes++;
    sb_save();
    return true;
}

// =================================================================
// Free-block bitmap
// =================================================================
static uint64_t block_alloc(void) {
    uint64_t data_blocks = g_sb.total_blocks - g_sb.data_start;
    uint64_t bits_per_block = (uint64_t)COP_BLOCK_SIZE * 8;
    uint8_t buf[COP_BLOCK_SIZE];

    for (uint64_t bb = 0; bb < g_sb.bitmap_blocks; bb++) {
        if (!block_read(g_sb.bitmap_start + bb, buf)) return UINT64_MAX;
        uint64_t base = bb * bits_per_block;
        for (uint64_t bit = 0; bit < bits_per_block; bit++) {
            uint64_t gbit = base + bit;
            if (gbit >= data_blocks) break;
            uint64_t byte = bit / 8;
            uint8_t mask = (uint8_t)(1u << (bit % 8));
            if (!(buf[byte] & mask)) {
                buf[byte] |= mask;
                if (!block_write(g_sb.bitmap_start + bb, buf)) return UINT64_MAX;
                g_sb.free_blocks--;
                sb_save();
                return g_sb.data_start + gbit;
            }
        }
    }
    return UINT64_MAX;
}

static bool block_free(uint64_t block) {
    if (block < g_sb.data_start || block >= g_sb.total_blocks) return false;
    uint64_t gbit = block - g_sb.data_start;
    uint64_t bits_per_block = (uint64_t)COP_BLOCK_SIZE * 8;
    uint64_t bb = gbit / bits_per_block, bit = gbit % bits_per_block;
    uint8_t buf[COP_BLOCK_SIZE];
    if (bb >= g_sb.bitmap_blocks) return false;
    if (!block_read(g_sb.bitmap_start + bb, buf)) return false;
    uint64_t byte = bit / 8;
    uint8_t mask = (uint8_t)(1u << (bit % 8));
    if (!(buf[byte] & mask)) return false; // double free guard
    buf[byte] &= (uint8_t)~mask;
    if (!block_write(g_sb.bitmap_start + bb, buf)) return false;
    g_sb.free_blocks++;
    sb_save();
    return true;
}

// =================================================================
// Path handling
// =================================================================
// Resolves a user-typed path (relative or absolute) against the
// current working directory into a normalized absolute path,
// collapsing "." and ".." components. Does NOT touch the disk.
static bool resolve_path(const char *arg, char *out, size_t outsz) {
    char raw[COP_MAX_PATH + 16];

    if (!arg || arg[0] == '\0') {
        strncpy(raw, g_cwd_path, sizeof(raw) - 1);
        raw[sizeof(raw) - 1] = '\0';
    } else if (arg[0] == '/') {
        strncpy(raw, arg, sizeof(raw) - 1);
        raw[sizeof(raw) - 1] = '\0';
    } else {
        if (strcmp(g_cwd_path, "/") == 0)
            snprintf(raw, sizeof(raw), "/%s", arg);
        else
            snprintf(raw, sizeof(raw), "%s/%s", g_cwd_path, arg);
    }

    char *stack[512];
    int depth = 0;

    char copy[COP_MAX_PATH];
    strncpy(copy, raw, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *tok = strtok(copy, "/");
    while (tok) {
        if (strcmp(tok, ".") == 0) {
            // no-op
        } else if (strcmp(tok, "..") == 0) {
            if (depth > 0) depth--;
        } else if (depth < 512) {
            stack[depth++] = tok;
        }
        tok = strtok(NULL, "/");
    }

    if (depth == 0) {
        if (outsz < 2) return false;
        strcpy(out, "/");
        return true;
    }

    size_t pos = 0;
    for (int i = 0; i < depth; i++) {
        size_t need = strlen(stack[i]) + 1;
        if (pos + need >= outsz) return false;
        out[pos++] = '/';
        memcpy(out + pos, stack[i], strlen(stack[i]));
        pos += strlen(stack[i]);
    }
    out[pos] = '\0';
    return true;
}

static bool split_path(const char *path, char *parent, char *name) {
    if (!path || path[0] != '/') return false;
    size_t len = strlen(path);
    if (len == 0 || len >= COP_MAX_PATH) return false;

    char tmp[COP_MAX_PATH];
    strcpy(tmp, path);
    size_t tl = strlen(tmp);
    while (tl > 1 && tmp[tl - 1] == '/') { tmp[tl - 1] = '\0'; tl--; }

    char *last = strrchr(tmp, '/');
    if (!last) return false;

    size_t nlen = strlen(last + 1);
    if (nlen == 0 || nlen >= COP_MAX_NAME_LEN) return false;
    strcpy(name, last + 1);

    if (last == tmp) {
        strcpy(parent, "/");
    } else {
        size_t l = (size_t)(last - tmp);
        memcpy(parent, tmp, l);
        parent[l] = '\0';
    }
    return true;
}

// =================================================================
// Directory entries
// =================================================================
typedef void (*dirent_cb)(const cop_dirent_t *de, void *ctx);

static bool dir_iterate(uint64_t inode_num, dirent_cb cb, void *ctx) {
    cop_inode_t dir;
    if (!inode_read(inode_num, &dir)) return false;
    if (dir.type != 2) return false;

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
        if (dir.blocks[i] == 0) continue;
        uint8_t buf[COP_BLOCK_SIZE];
        if (!block_read(dir.blocks[i], buf)) continue;
        uint64_t off = 0;
        while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
            cop_dirent_t *de = (cop_dirent_t *)(buf + off);
            if (de->inode != 0) cb(de, ctx);
            off += sizeof(cop_dirent_t);
        }
    }
    return true;
}

static bool fs_lookup(const char *path, uint64_t *out_inode) {
    if (!path || path[0] != '/') return false;
    if (strcmp(path, "/") == 0) { *out_inode = g_sb.root_inode; return true; }

    uint64_t cur = g_sb.root_inode;
    const char *p = path + 1;
    char comp[COP_MAX_NAME_LEN];

    while (*p) {
        uint64_t len = 0;
        while (p[len] != '/' && p[len] != '\0') {
            len++;
            if (len >= COP_MAX_NAME_LEN) return false;
        }
        memcpy(comp, p, len);
        comp[len] = '\0';

        cop_inode_t dir;
        if (!inode_read(cur, &dir)) return false;
        if (dir.type != 2) return false;

        bool found = false;
        uint64_t found_inode = 0;

        for (int i = 0; i < COP_DIRECT_BLOCKS && !found; i++) {
            if (dir.blocks[i] == 0) continue;
            uint8_t buf[COP_BLOCK_SIZE];
            if (!block_read(dir.blocks[i], buf)) continue;
            uint64_t off = 0;
            while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
                cop_dirent_t *de = (cop_dirent_t *)(buf + off);
                if (de->inode != 0 && strcmp(de->name, comp) == 0) {
                    found = true;
                    found_inode = de->inode;
                    break;
                }
                off += sizeof(cop_dirent_t);
            }
        }

        if (!found) return false;
        cur = found_inode;
        p += len;
        if (*p == '/') p++;
    }

    *out_inode = cur;
    return true;
}

static bool dir_add_entry(uint64_t parent, const char *name, uint64_t inode, uint8_t type) {
    if (!name || strlen(name) == 0 || strlen(name) >= COP_MAX_NAME_LEN) return false;

    cop_inode_t dir;
    if (!inode_read(parent, &dir)) return false;
    if (dir.type != 2) return false;

    cop_dirent_t entry;
    memset(&entry, 0, sizeof(entry));
    entry.inode = inode;
    entry.type = type;
    entry.name_length = (uint16_t)strlen(name);
    strcpy(entry.name, name);

    uint64_t epb = COP_BLOCK_SIZE / sizeof(cop_dirent_t);
    uint64_t eidx = dir.size / sizeof(cop_dirent_t);
    uint64_t bidx = eidx / epb;
    uint64_t boff = (eidx % epb) * sizeof(cop_dirent_t);

    if (bidx >= COP_DIRECT_BLOCKS) return false;

    uint64_t block = dir.blocks[bidx];
    uint8_t buf[COP_BLOCK_SIZE];

    if (block == 0) {
        block = block_alloc();
        if (block == UINT64_MAX) return false;
        dir.blocks[bidx] = block;
        memset(buf, 0, COP_BLOCK_SIZE);
    } else {
        if (!block_read(block, buf)) return false;
    }

    memcpy(buf + boff, &entry, sizeof(entry));
    if (!block_write(block, buf)) return false;

    dir.size += sizeof(entry);
    return inode_write(parent, &dir);
}

static bool dir_remove_entry(uint64_t parent, const char *name) {
    cop_inode_t dir;
    if (!inode_read(parent, &dir)) return false;
    if (dir.type != 2) return false;

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
        if (dir.blocks[i] == 0) continue;
        uint8_t buf[COP_BLOCK_SIZE];
        if (!block_read(dir.blocks[i], buf)) continue;
        uint64_t off = 0;
        while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
            cop_dirent_t *de = (cop_dirent_t *)(buf + off);
            if (de->inode != 0 && strcmp(de->name, name) == 0) {
                memset(de, 0, sizeof(cop_dirent_t));
                return block_write(dir.blocks[i], buf);
            }
            off += sizeof(cop_dirent_t);
        }
    }
    return false;
}

static bool dir_rename_entry(uint64_t parent, const char *oldname, const char *newname) {
    if (strlen(newname) == 0 || strlen(newname) >= COP_MAX_NAME_LEN) return false;

    cop_inode_t dir;
    if (!inode_read(parent, &dir)) return false;

    // refuse if newname already exists
    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
        if (dir.blocks[i] == 0) continue;
        uint8_t buf[COP_BLOCK_SIZE];
        if (!block_read(dir.blocks[i], buf)) continue;
        uint64_t off = 0;
        while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
            cop_dirent_t *de = (cop_dirent_t *)(buf + off);
            if (de->inode != 0 && strcmp(de->name, newname) == 0) return false;
            off += sizeof(cop_dirent_t);
        }
    }

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
        if (dir.blocks[i] == 0) continue;
        uint8_t buf[COP_BLOCK_SIZE];
        if (!block_read(dir.blocks[i], buf)) continue;
        uint64_t off = 0;
        while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
            cop_dirent_t *de = (cop_dirent_t *)(buf + off);
            if (de->inode != 0 && strcmp(de->name, oldname) == 0) {
                memset(de->name, 0, COP_MAX_NAME_LEN);
                strcpy(de->name, newname);
                de->name_length = (uint16_t)strlen(newname);
                return block_write(dir.blocks[i], buf);
            }
            off += sizeof(cop_dirent_t);
        }
    }
    return false;
}

static bool dir_is_empty(uint64_t inode_num) {
    cop_inode_t dir;
    if (!inode_read(inode_num, &dir)) return false;
    if (dir.type != 2) return false;
    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
        if (dir.blocks[i] == 0) continue;
        uint8_t buf[COP_BLOCK_SIZE];
        if (!block_read(dir.blocks[i], buf)) continue;
        uint64_t off = 0;
        while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
            cop_dirent_t *de = (cop_dirent_t *)(buf + off);
            if (de->inode != 0) return false;
            off += sizeof(cop_dirent_t);
        }
    }
    return true;
}

// =================================================================
// File data I/O
// =================================================================
#define COP_MAX_FILE_SIZE ((uint64_t)COP_DIRECT_BLOCKS * COP_BLOCK_SIZE)

static bool file_write(uint64_t inode_num, const void *buf, uint64_t size) {
    if (size > COP_MAX_FILE_SIZE) return false;

    cop_inode_t nd;
    if (!inode_read(inode_num, &nd)) return false;
    if (nd.type != 1) return false;

    uint64_t old_blocks = (nd.size + COP_BLOCK_SIZE - 1) / COP_BLOCK_SIZE;
    uint64_t new_blocks = (size + COP_BLOCK_SIZE - 1) / COP_BLOCK_SIZE;

    uint64_t remaining = size, off = 0, bidx = 0;
    while (remaining > 0) {
        uint64_t block = nd.blocks[bidx];
        if (block == 0) {
            block = block_alloc();
            if (block == UINT64_MAX) return false;
            nd.blocks[bidx] = block;
        }
        uint8_t bbuf[COP_BLOCK_SIZE];
        memset(bbuf, 0, COP_BLOCK_SIZE);
        uint64_t wsize = remaining > COP_BLOCK_SIZE ? COP_BLOCK_SIZE : remaining;
        memcpy(bbuf, (const uint8_t *)buf + off, wsize);
        if (!block_write(block, bbuf)) return false;
        remaining -= wsize;
        off += wsize;
        bidx++;
    }

    for (uint64_t i = new_blocks; i < old_blocks && i < COP_DIRECT_BLOCKS; i++) {
        if (nd.blocks[i] != 0) { block_free(nd.blocks[i]); nd.blocks[i] = 0; }
    }

    nd.size = size;
    return inode_write(inode_num, &nd);
}

static bool file_read_all(uint64_t inode_num, uint8_t **out_buf, uint64_t *out_size) {
    cop_inode_t nd;
    if (!inode_read(inode_num, &nd)) return false;
    if (nd.type != 1) return false;

    uint8_t *buf = malloc(nd.size ? nd.size : 1);
    if (!buf) return false;

    uint64_t remaining = nd.size, off = 0, bidx = 0;
    while (remaining > 0 && bidx < COP_DIRECT_BLOCKS) {
        uint64_t block = nd.blocks[bidx];
        if (block == 0) break;
        uint8_t bbuf[COP_BLOCK_SIZE];
        if (!block_read(block, bbuf)) { free(buf); return false; }
        uint64_t rsize = remaining > COP_BLOCK_SIZE ? COP_BLOCK_SIZE : remaining;
        memcpy(buf + off, bbuf, rsize);
        remaining -= rsize;
        off += rsize;
        bidx++;
    }

    *out_buf = buf;
    *out_size = nd.size;
    return true;
}

// =================================================================
// High-level create / delete
// =================================================================
static bool fs_create(const char *path, uint8_t type) {
    char parent[COP_MAX_PATH], name[COP_MAX_NAME_LEN];
    if (!split_path(path, parent, name)) return false;
    if (strlen(name) == 0) return false;

    uint64_t existing;
    if (fs_lookup(path, &existing)) return false;

    uint64_t parent_inode;
    if (!fs_lookup(parent, &parent_inode)) return false;

    cop_inode_t pnode;
    if (!inode_read(parent_inode, &pnode)) return false;
    if (pnode.type != 2) return false;

    uint64_t new_inode = inode_alloc(type);
    if (new_inode == UINT64_MAX) return false;

    cop_inode_t nd;
    memset(&nd, 0, sizeof(nd));
    nd.type = type;
    nd.size = 0;
    nd.created = (uint64_t)time(NULL);
    if (!inode_write(new_inode, &nd)) { inode_free(new_inode); return false; }

    if (!dir_add_entry(parent_inode, name, new_inode, type)) {
        inode_free(new_inode);
        return false;
    }
    return true;
}

static bool fs_delete_recursive(const char *path) {
    uint64_t inode_num;
    if (!fs_lookup(path, &inode_num)) return false;
    if (inode_num == g_sb.root_inode) return false;

    cop_inode_t nd;
    if (!inode_read(inode_num, &nd)) return false;

    if (nd.type == 2) {
        for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
            if (nd.blocks[i] == 0) continue;
            uint8_t buf[COP_BLOCK_SIZE];
            if (!block_read(nd.blocks[i], buf)) continue;
            uint64_t off = 0;
            while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
                cop_dirent_t *de = (cop_dirent_t *)(buf + off);
                if (de->inode != 0) {
                    char child[COP_MAX_PATH];
                    if (strcmp(path, "/") == 0)
                        snprintf(child, sizeof(child), "/%s", de->name);
                    else
                        snprintf(child, sizeof(child), "%s/%s", path, de->name);
                    fs_delete_recursive(child);
                }
                off += sizeof(cop_dirent_t);
            }
        }
    }

    char parent[COP_MAX_PATH], name[COP_MAX_NAME_LEN];
    if (!split_path(path, parent, name)) return false;

    uint64_t parent_inode;
    if (fs_lookup(parent, &parent_inode))
        dir_remove_entry(parent_inode, name);

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++)
        if (nd.blocks[i] != 0) block_free(nd.blocks[i]);

    return inode_free(inode_num);
}

// =================================================================
// Small helpers
// =================================================================
static const char *type_str(uint8_t t) {
    switch (t) {
        case 1: return "file";
        case 2: return "dir";
        default: return "?";
    }
}

// created uses the same bitpacked layout as cop_get_time() in the
// kernel: year<<26 | month<<22 | day<<17 | hour<<12 | min<<6 | sec
static void print_created(uint64_t ts) {
    uint32_t year  = (uint32_t)((ts >> 26) & 0xFFFF);
    uint32_t month = (uint32_t)((ts >> 22) & 0x0F);
    uint32_t day   = (uint32_t)((ts >> 17) & 0x1F);
    uint32_t hour  = (uint32_t)((ts >> 12) & 0x1F);
    uint32_t min   = (uint32_t)((ts >> 6)  & 0x3F);
    uint32_t sec   = (uint32_t)(ts & 0x3F);
    if (year >= 2000 && year < 2100)
        printf("%04u-%02u-%02u %02u:%02u:%02u", year, month, day, hour, min, sec);
    else
        printf("%llu (raw, not a cop_get_time() timestamp)", (unsigned long long)ts);
}

// Splits a line into an argv-style array. No quoting support.
static int tokenize(char *line, char **argv, int max) {
    int argc = 0;
    char *tok = strtok(line, " \t");
    while (tok && argc < max) {
        argv[argc++] = tok;
        tok = strtok(NULL, " \t");
    }
    return argc;
}

// =================================================================
// Command: ls
// =================================================================
struct ls_ctx { bool long_fmt; int count; };

static void ls_cb(const cop_dirent_t *de, void *ctxp) {
    struct ls_ctx *ctx = (struct ls_ctx *)ctxp;
    ctx->count++;
    if (ctx->long_fmt) {
        cop_inode_t nd;
        if (inode_read(de->inode, &nd))
            printf("%-4s %10llu  inode=%-6llu %s\n",
                   type_str(de->type), (unsigned long long)nd.size,
                   (unsigned long long)de->inode, de->name);
        else
            printf("%-4s %10s  inode=%-6llu %s\n", type_str(de->type), "?",
                   (unsigned long long)de->inode, de->name);
    } else {
        printf("%s%s\n", de->name, de->type == 2 ? "/" : "");
    }
}

static int cmd_ls(int argc, char **argv) {
    static struct option opts[] = {
        {"all",  no_argument, 0, 'a'},
        {"long", no_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool long_fmt = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "alh", opts, NULL)) != -1) {
        switch (c) {
            case 'l': long_fmt = true; break;
            case 'a': break; // COPFS has no hidden entries; kept for symmetry
            case 'h':
            default:
                printf("usage: ls [-l|--long] [-a|--all] [-h|--help] [path]\n");
                return 0;
        }
    }
    char path[COP_MAX_PATH];
    const char *arg = optind < argc ? argv[optind] : NULL;
    if (!resolve_path(arg, path, sizeof(path))) { fprintf(stderr, "ls: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "ls: %s: no such entry\n", path); return 1; }

    cop_inode_t nd;
    if (!inode_read(inode, &nd) || nd.type != 2) { fprintf(stderr, "ls: %s: not a directory\n", path); return 1; }

    struct ls_ctx ctx = { long_fmt, 0 };
    dir_iterate(inode, ls_cb, &ctx);
    if (long_fmt) printf("%d entr%s\n", ctx.count, ctx.count == 1 ? "y" : "ies");
    return 0;
}

// =================================================================
// Command: cd / pwd
// =================================================================
static int cmd_cd(int argc, char **argv) {
    static struct option opts[] = { {"help", no_argument, 0, 'h'}, {0,0,0,0} };
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "h", opts, NULL)) != -1) {
        printf("usage: cd [-h|--help] [name|..|/]\n");
        return 0;
        (void)c;
    }
    const char *arg = optind < argc ? argv[optind] : "/";
    char path[COP_MAX_PATH];
    if (!resolve_path(arg, path, sizeof(path))) { fprintf(stderr, "cd: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "cd: %s: no such directory\n", path); return 1; }

    cop_inode_t nd;
    if (!inode_read(inode, &nd) || nd.type != 2) { fprintf(stderr, "cd: %s: not a directory\n", path); return 1; }

    g_cwd_inode = inode;
    strncpy(g_cwd_path, path, sizeof(g_cwd_path) - 1);
    g_cwd_path[sizeof(g_cwd_path) - 1] = '\0';
    return 0;
}

static int cmd_pwd(int argc, char **argv) {
    static struct option opts[] = { {"help", no_argument, 0, 'h'}, {0,0,0,0} };
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "h", opts, NULL)) != -1) {
        printf("usage: pwd [-h|--help]\n");
        return 0;
        (void)c;
    }
    printf("%s\n", g_cwd_path);
    return 0;
}

// =================================================================
// Command: tree
// =================================================================
struct tree_ctx { int depth; int max_depth; };

static void tree_print(uint64_t inode_num, int depth, int max_depth) {
    cop_inode_t dir;
    if (!inode_read(inode_num, &dir) || dir.type != 2) return;

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
        if (dir.blocks[i] == 0) continue;
        uint8_t buf[COP_BLOCK_SIZE];
        if (!block_read(dir.blocks[i], buf)) continue;
        uint64_t off = 0;
        while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
            cop_dirent_t *de = (cop_dirent_t *)(buf + off);
            if (de->inode != 0) {
                for (int d = 0; d < depth; d++) printf("  ");
                printf("%s%s\n", de->name, de->type == 2 ? "/" : "");
                if (de->type == 2 && (max_depth < 0 || depth + 1 < max_depth))
                    tree_print(de->inode, depth + 1, max_depth);
            }
            off += sizeof(cop_dirent_t);
        }
    }
}

static int cmd_tree(int argc, char **argv) {
    static struct option opts[] = {
        {"depth", required_argument, 0, 'd'},
        {"help",  no_argument,       0, 'h'},
        {0,0,0,0}
    };
    int max_depth = -1;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "d:h", opts, NULL)) != -1) {
        switch (c) {
            case 'd': max_depth = atoi(optarg); break;
            case 'h':
            default:
                printf("usage: tree [-d|--depth N] [-h|--help] [path]\n");
                return 0;
        }
    }
    char path[COP_MAX_PATH];
    const char *arg = optind < argc ? argv[optind] : NULL;
    if (!resolve_path(arg, path, sizeof(path))) { fprintf(stderr, "tree: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "tree: %s: no such entry\n", path); return 1; }

    printf("%s\n", path);
    tree_print(inode, 1, max_depth);
    return 0;
}

// =================================================================
// Command: find
// =================================================================
struct find_ctx { const char *needle; bool icase; int type_filter; };

static void find_walk(uint64_t inode_num, const char *prefix, struct find_ctx *ctx) {
    cop_inode_t dir;
    if (!inode_read(inode_num, &dir) || dir.type != 2) return;

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) {
        if (dir.blocks[i] == 0) continue;
        uint8_t buf[COP_BLOCK_SIZE];
        if (!block_read(dir.blocks[i], buf)) continue;
        uint64_t off = 0;
        while (off + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE) {
            cop_dirent_t *de = (cop_dirent_t *)(buf + off);
            if (de->inode != 0) {
                char full[COP_MAX_PATH];
                if (strcmp(prefix, "/") == 0)
                    snprintf(full, sizeof(full), "/%s", de->name);
                else
                    snprintf(full, sizeof(full), "%s/%s", prefix, de->name);

                bool name_matches = ctx->icase ? (strcasestr(de->name, ctx->needle) != NULL)
                                                : (strstr(de->name, ctx->needle) != NULL);
                bool type_matches = ctx->type_filter == 0 || ctx->type_filter == de->type;

                if (name_matches && type_matches)
                    printf("%s\n", full);

                if (de->type == 2)
                    find_walk(de->inode, full, ctx);
            }
            off += sizeof(cop_dirent_t);
        }
    }
}

static int cmd_find(int argc, char **argv) {
    static struct option opts[] = {
        {"ignore-case", no_argument,       0, 'i'},
        {"type",        required_argument, 0, 't'},
        {"help",        no_argument,       0, 'h'},
        {0,0,0,0}
    };
    bool icase = false;
    int type_filter = 0;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "it:h", opts, NULL)) != -1) {
        switch (c) {
            case 'i': icase = true; break;
            case 't':
                if (optarg[0] == 'f') type_filter = 1;
                else if (optarg[0] == 'd') type_filter = 2;
                break;
            case 'h':
            default:
                printf("usage: find [-i|--ignore-case] [-t|--type f|d] [-h|--help] <name>\n");
                return 0;
        }
    }
    if (optind >= argc) { fprintf(stderr, "find: missing <name>\n"); return 1; }

    struct find_ctx ctx = { argv[optind], icase, type_filter };
    find_walk(g_sb.root_inode, "/", &ctx);
    return 0;
}

// =================================================================
// Command: cat / head / hexdump
// =================================================================
static int cmd_cat(int argc, char **argv) {
    static struct option opts[] = {
        {"number", no_argument, 0, 'n'},
        {"help",   no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool number = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "nh", opts, NULL)) != -1) {
        switch (c) {
            case 'n': number = true; break;
            case 'h':
            default:
                printf("usage: cat [-n|--number] [-h|--help] <name>\n");
                return 0;
        }
    }
    if (optind >= argc) { fprintf(stderr, "cat: missing <name>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "cat: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "cat: %s: no such file\n", path); return 1; }

    uint8_t *buf; uint64_t size;
    if (!file_read_all(inode, &buf, &size)) { fprintf(stderr, "cat: %s: read failed\n", path); return 1; }

    if (!number) {
        fwrite(buf, 1, size, stdout);
    } else {
        int line = 1;
        printf("%6d\t", line++);
        for (uint64_t i = 0; i < size; i++) {
            putchar(buf[i]);
            if (buf[i] == '\n' && i + 1 < size) printf("%6d\t", line++);
        }
    }
    if (size == 0 || buf[size - 1] != '\n') printf("\n");
    free(buf);
    return 0;
}

static int cmd_head(int argc, char **argv) {
    static struct option opts[] = {
        {"lines", required_argument, 0, 'n'},
        {"bytes", required_argument, 0, 'c'},
        {"help",  no_argument,       0, 'h'},
        {0,0,0,0}
    };
    long lines = 10, bytes = -1;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "n:c:h", opts, NULL)) != -1) {
        switch (c) {
            case 'n': lines = atol(optarg); break;
            case 'c': bytes = atol(optarg); break;
            case 'h':
            default:
                printf("usage: head [-n|--lines N] [-c|--bytes N] [-h|--help] <name> [n]\n");
                return 0;
        }
    }
    if (optind >= argc) { fprintf(stderr, "head: missing <name>\n"); return 1; }
    const char *name_arg = argv[optind];
    // legacy positional [n] for parity with the original signature
    if (optind + 1 < argc) lines = atol(argv[optind + 1]);

    char path[COP_MAX_PATH];
    if (!resolve_path(name_arg, path, sizeof(path))) { fprintf(stderr, "head: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "head: %s: no such file\n", path); return 1; }

    uint8_t *buf; uint64_t size;
    if (!file_read_all(inode, &buf, &size)) { fprintf(stderr, "head: %s: read failed\n", path); return 1; }

    if (bytes >= 0) {
        uint64_t n = (uint64_t)bytes < size ? (uint64_t)bytes : size;
        fwrite(buf, 1, n, stdout);
        printf("\n");
    } else {
        long shown = 0;
        for (uint64_t i = 0; i < size && shown < lines; i++) {
            putchar(buf[i]);
            if (buf[i] == '\n') shown++;
        }
        if (shown < lines) printf("\n");
    }
    free(buf);
    return 0;
}

static int cmd_hexdump(int argc, char **argv) {
    static struct option opts[] = {
        {"length", required_argument, 0, 'n'},
        {"offset", required_argument, 0, 'o'},
        {"help",   no_argument,       0, 'h'},
        {0,0,0,0}
    };
    long limit = -1, start_off = 0;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "n:o:h", opts, NULL)) != -1) {
        switch (c) {
            case 'n': limit = atol(optarg); break;
            case 'o': start_off = atol(optarg); break;
            case 'h':
            default:
                printf("usage: hexdump [-n|--length N] [-o|--offset N] [-h|--help] <name>\n");
                return 0;
        }
    }
    if (optind >= argc) { fprintf(stderr, "hexdump: missing <name>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "hexdump: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "hexdump: %s: no such file\n", path); return 1; }

    uint8_t *buf; uint64_t size;
    if (!file_read_all(inode, &buf, &size)) { fprintf(stderr, "hexdump: %s: read failed\n", path); return 1; }

    uint64_t off = start_off > 0 ? (uint64_t)start_off : 0;
    uint64_t end = (limit >= 0 && off + (uint64_t)limit < size) ? off + (uint64_t)limit : size;

    for (uint64_t base = off; base < end; base += 16) {
        printf("%08llx  ", (unsigned long long)base);
        uint64_t row_end = base + 16 < end ? base + 16 : end;
        for (uint64_t i = base; i < base + 16; i++) {
            if (i < row_end) printf("%02x ", buf[i]);
            else printf("   ");
            if (i - base == 7) printf(" ");
        }
        printf(" |");
        for (uint64_t i = base; i < row_end; i++)
            putchar(isprint(buf[i]) ? buf[i] : '.');
        printf("|\n");
    }
    free(buf);
    return 0;
}

// =================================================================
// Command: stat / df
// =================================================================
static int cmd_stat(int argc, char **argv) {
    static struct option opts[] = { {"help", no_argument, 0, 'h'}, {0,0,0,0} };
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "h", opts, NULL)) != -1) {
        printf("usage: stat [-h|--help] <name>\n");
        return 0;
        (void)c;
    }
    if (optind >= argc) { fprintf(stderr, "stat: missing <name>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "stat: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "stat: %s: no such entry\n", path); return 1; }

    cop_inode_t nd;
    if (!inode_read(inode, &nd)) { fprintf(stderr, "stat: read failed\n"); return 1; }

    printf("  path:    %s\n", path);
    printf("  inode:   %llu\n", (unsigned long long)inode);
    printf("  type:    %s\n", type_str(nd.type));
    printf("  size:    %llu bytes\n", (unsigned long long)nd.size);
    printf("  created: ");
    print_created(nd.created);
    printf("\n");
    int used = 0;
    for (int i = 0; i < COP_DIRECT_BLOCKS; i++) if (nd.blocks[i]) used++;
    printf("  blocks:  %d used of %d direct (", used, COP_DIRECT_BLOCKS);
    for (int i = 0; i < used; i++) printf("%s%llu", i ? "," : "", (unsigned long long)nd.blocks[i]);
    printf(")\n");
    return 0;
}

static int cmd_df(int argc, char **argv) {
    static struct option opts[] = {
        {"human", no_argument, 0, 'h'},
        {"help",  no_argument, 0, 'H'}, // avoid colliding with -h/human
        {0,0,0,0}
    };
    bool human = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "hH", opts, NULL)) != -1) {
        switch (c) {
            case 'h': human = true; break;
            case 'H':
            default:
                printf("usage: df [-h|--human] [--help]\n");
                return 0;
        }
    }
    uint64_t used_blocks = g_sb.total_blocks - g_sb.data_start - g_sb.free_blocks;
    uint64_t total_data  = g_sb.total_blocks - g_sb.data_start;
    uint64_t used_inodes = g_sb.total_inodes - g_sb.free_inodes;

    if (human) {
        double bs = (double)COP_BLOCK_SIZE;
        printf("blocks: %.2f MB used / %.2f MB total (%.1f%% used)\n",
               used_blocks * bs / 1e6, total_data * bs / 1e6,
               total_data ? 100.0 * used_blocks / total_data : 0.0);
    } else {
        printf("blocks: %llu / %llu used\n", (unsigned long long)used_blocks, (unsigned long long)total_data);
    }
    printf("inodes: %llu / %llu used\n", (unsigned long long)used_inodes, (unsigned long long)g_sb.total_inodes);
    printf("block size: %u bytes\n", g_sb.block_size);
    return 0;
}

// =================================================================
// Command: get / put
// =================================================================
static int cmd_get(int argc, char **argv) {
    static struct option opts[] = {
        {"force", no_argument, 0, 'f'},
        {"help",  no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool force = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "fh", opts, NULL)) != -1) {
        switch (c) {
            case 'f': force = true; break;
            case 'h':
            default:
                printf("usage: get [-f|--force] [-h|--help] <name> <hostpath>\n");
                return 0;
        }
    }
    if (optind + 1 >= argc) { fprintf(stderr, "get: usage: get <name> <hostpath>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "get: bad path\n"); return 1; }
    const char *hostpath = argv[optind + 1];

    if (!force) {
        FILE *probe = fopen(hostpath, "rb");
        if (probe) { fclose(probe); fprintf(stderr, "get: %s exists, use -f to overwrite\n", hostpath); return 1; }
    }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "get: %s: no such file\n", path); return 1; }

    uint8_t *buf; uint64_t size;
    if (!file_read_all(inode, &buf, &size)) { fprintf(stderr, "get: read failed\n"); return 1; }

    FILE *out = fopen(hostpath, "wb");
    if (!out) { fprintf(stderr, "get: cannot open %s: %s\n", hostpath, strerror(errno)); free(buf); return 1; }
    fwrite(buf, 1, size, out);
    fclose(out);
    free(buf);
    printf("get: wrote %llu bytes to %s\n", (unsigned long long)size, hostpath);
    return 0;
}

static int cmd_put(int argc, char **argv) {
    static struct option opts[] = {
        {"force", no_argument, 0, 'f'},
        {"help",  no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool force = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "fh", opts, NULL)) != -1) {
        switch (c) {
            case 'f': force = true; break;
            case 'h':
            default:
                printf("usage: put [-f|--force] [-h|--help] <hostpath> <name>\n");
                return 0;
        }
    }
    if (optind + 1 >= argc) { fprintf(stderr, "put: usage: put <hostpath> <name>\n"); return 1; }

    const char *hostpath = argv[optind];
    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind + 1], path, sizeof(path))) { fprintf(stderr, "put: bad path\n"); return 1; }

    FILE *in = fopen(hostpath, "rb");
    if (!in) { fprintf(stderr, "put: cannot open %s: %s\n", hostpath, strerror(errno)); return 1; }
    fseeko(in, 0, SEEK_END);
    off_t hsize = ftello(in);
    fseeko(in, 0, SEEK_SET);

    if ((uint64_t)hsize > COP_MAX_FILE_SIZE) {
        fprintf(stderr, "put: warning - %s is %lld bytes, truncating to max file size %llu\n",
                hostpath, (long long)hsize, (unsigned long long)COP_MAX_FILE_SIZE);
        hsize = (off_t)COP_MAX_FILE_SIZE;
    }

    uint8_t *buf = malloc(hsize ? (size_t)hsize : 1);
    if (!buf) { fclose(in); fprintf(stderr, "put: out of memory\n"); return 1; }
    size_t rd = fread(buf, 1, (size_t)hsize, in);
    fclose(in);

    uint64_t existing;
    bool exists = fs_lookup(path, &existing);
    if (exists && !force) { fprintf(stderr, "put: %s exists, use -f to overwrite\n", path); free(buf); return 1; }

    if (!exists) {
        if (!fs_create(path, 1)) { fprintf(stderr, "put: create failed\n"); free(buf); return 1; }
        fs_lookup(path, &existing);
    }

    bool ok = file_write(existing, buf, rd);
    free(buf);
    if (!ok) { fprintf(stderr, "put: write failed\n"); return 1; }

    printf("put: wrote %zu bytes to %s\n", rd, path);
    return 0;
}

// =================================================================
// Command: mkdir / touch / rm / rmdir / mv / cp
// =================================================================
static int cmd_mkdir(int argc, char **argv) {
    static struct option opts[] = {
        {"parents", no_argument, 0, 'p'},
        {"help",    no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool parents = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "ph", opts, NULL)) != -1) {
        switch (c) {
            case 'p': parents = true; break;
            case 'h':
            default:
                printf("usage: mkdir [-p|--parents] [-h|--help] <name>\n");
                return 0;
        }
    }
    if (optind >= argc) { fprintf(stderr, "mkdir: missing <name>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "mkdir: bad path\n"); return 1; }

    if (!parents) {
        if (!fs_create(path, 2)) { fprintf(stderr, "mkdir: %s: failed (exists or bad parent)\n", path); return 1; }
        return 0;
    }

    // walk and create each missing intermediate component
    char partial[COP_MAX_PATH] = "";
    char copy[COP_MAX_PATH];
    strncpy(copy, path, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *tok = strtok(copy, "/");
    while (tok) {
        strcat(partial, "/");
        strcat(partial, tok);
        uint64_t existing;
        if (!fs_lookup(partial, &existing)) {
            if (!fs_create(partial, 2)) { fprintf(stderr, "mkdir: failed at %s\n", partial); return 1; }
        }
        tok = strtok(NULL, "/");
    }
    return 0;
}

static int cmd_touch(int argc, char **argv) {
    static struct option opts[] = { {"help", no_argument, 0, 'h'}, {0,0,0,0} };
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "h", opts, NULL)) != -1) {
        printf("usage: touch [-h|--help] <name>\n");
        return 0;
        (void)c;
    }
    if (optind >= argc) { fprintf(stderr, "touch: missing <name>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "touch: bad path\n"); return 1; }

    uint64_t inode;
    if (fs_lookup(path, &inode)) {
        cop_inode_t nd;
        if (!inode_read(inode, &nd)) return 1;
        nd.created = (uint64_t)time(NULL);
        inode_write(inode, &nd);
        return 0;
    }
    if (!fs_create(path, 1)) { fprintf(stderr, "touch: %s: failed\n", path); return 1; }
    return 0;
}

static int cmd_rm(int argc, char **argv) {
    static struct option opts[] = {
        {"force", no_argument, 0, 'f'},
        {"help",  no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool force = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "fh", opts, NULL)) != -1) {
        switch (c) {
            case 'f': force = true; break;
            case 'h':
            default:
                printf("usage: rm [-f|--force] [-h|--help] <name>\n");
                return 0;
        }
    }
    if (optind >= argc) { fprintf(stderr, "rm: missing <name>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "rm: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) {
        if (force) return 0;
        fprintf(stderr, "rm: %s: no such file\n", path);
        return 1;
    }
    cop_inode_t nd;
    if (!inode_read(inode, &nd)) return 1;
    if (nd.type == 2) { fprintf(stderr, "rm: %s: is a directory (use rmdir)\n", path); return 1; }

    char parent[COP_MAX_PATH], name[COP_MAX_NAME_LEN];
    if (!split_path(path, parent, name)) return 1;
    uint64_t parent_inode;
    if (!fs_lookup(parent, &parent_inode)) return 1;
    if (!dir_remove_entry(parent_inode, name)) { fprintf(stderr, "rm: remove failed\n"); return 1; }

    for (int i = 0; i < COP_DIRECT_BLOCKS; i++)
        if (nd.blocks[i]) block_free(nd.blocks[i]);
    inode_free(inode);
    return 0;
}

static int cmd_rmdir(int argc, char **argv) {
    static struct option opts[] = {
        {"force", no_argument, 0, 'f'},
        {"help",  no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool force = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "fh", opts, NULL)) != -1) {
        switch (c) {
            case 'f': force = true; break;
            case 'h':
            default:
                printf("usage: rmdir [-f|--force] [-h|--help] <name>\n");
                printf("  -f also removes non-empty directories, recursively\n");
                return 0;
        }
    }
    if (optind >= argc) { fprintf(stderr, "rmdir: missing <name>\n"); return 1; }

    char path[COP_MAX_PATH];
    if (!resolve_path(argv[optind], path, sizeof(path))) { fprintf(stderr, "rmdir: bad path\n"); return 1; }

    uint64_t inode;
    if (!fs_lookup(path, &inode)) { fprintf(stderr, "rmdir: %s: no such directory\n", path); return 1; }

    cop_inode_t nd;
    if (!inode_read(inode, &nd) || nd.type != 2) { fprintf(stderr, "rmdir: %s: not a directory\n", path); return 1; }

    if (!force && !dir_is_empty(inode)) {
        fprintf(stderr, "rmdir: %s: not empty (use -f to force)\n", path);
        return 1;
    }
    if (!fs_delete_recursive(path)) { fprintf(stderr, "rmdir: failed\n"); return 1; }
    return 0;
}

static int cmd_mv(int argc, char **argv) {
    static struct option opts[] = {
        {"force", no_argument, 0, 'f'},
        {"help",  no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool force = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "fh", opts, NULL)) != -1) {
        switch (c) {
            case 'f': force = true; break;
            case 'h':
            default:
                printf("usage: mv [-f|--force] [-h|--help] <old> <new>\n");
                printf("  renames within the current directory only\n");
                return 0;
        }
    }
    if (optind + 1 >= argc) { fprintf(stderr, "mv: usage: mv <old> <new>\n"); return 1; }

    char oldpath[COP_MAX_PATH], newpath[COP_MAX_PATH];
    if (!resolve_path(argv[optind], oldpath, sizeof(oldpath))) { fprintf(stderr, "mv: bad path\n"); return 1; }
    if (!resolve_path(argv[optind + 1], newpath, sizeof(newpath))) { fprintf(stderr, "mv: bad path\n"); return 1; }

    char old_parent[COP_MAX_PATH], old_name[COP_MAX_NAME_LEN];
    char new_parent[COP_MAX_PATH], new_name[COP_MAX_NAME_LEN];
    if (!split_path(oldpath, old_parent, old_name)) { fprintf(stderr, "mv: %s: bad path\n", oldpath); return 1; }
    if (!split_path(newpath, new_parent, new_name)) { fprintf(stderr, "mv: %s: bad path\n", newpath); return 1; }

    if (strcmp(old_parent, new_parent) != 0) {
        fprintf(stderr, "mv: only same-directory renames are supported (%s vs %s)\n", old_parent, new_parent);
        return 1;
    }

    uint64_t parent_inode;
    if (!fs_lookup(old_parent, &parent_inode)) { fprintf(stderr, "mv: %s: no such directory\n", old_parent); return 1; }

    if (force) {
        uint64_t existing;
        if (fs_lookup(newpath, &existing) && existing != g_sb.root_inode)
            fs_delete_recursive(newpath);
    }

    if (!dir_rename_entry(parent_inode, old_name, new_name)) {
        fprintf(stderr, "mv: rename failed (missing source or name in use)\n");
        return 1;
    }
    return 0;
}

static int cmd_cp(int argc, char **argv) {
    static struct option opts[] = {
        {"force", no_argument, 0, 'f'},
        {"help",  no_argument, 0, 'h'},
        {0,0,0,0}
    };
    bool force = false;
    int c; optind = 0; opterr = 0;
    while ((c = getopt_long(argc, argv, "fh", opts, NULL)) != -1) {
        switch (c) {
            case 'f': force = true; break;
            case 'h':
            default:
                printf("usage: cp [-f|--force] [-h|--help] <name> <newname>\n");
                return 0;
        }
    }
    if (optind + 1 >= argc) { fprintf(stderr, "cp: usage: cp <name> <newname>\n"); return 1; }

    char srcpath[COP_MAX_PATH], dstpath[COP_MAX_PATH];
    if (!resolve_path(argv[optind], srcpath, sizeof(srcpath))) { fprintf(stderr, "cp: bad path\n"); return 1; }
    if (!resolve_path(argv[optind + 1], dstpath, sizeof(dstpath))) { fprintf(stderr, "cp: bad path\n"); return 1; }

    uint64_t src_inode;
    if (!fs_lookup(srcpath, &src_inode)) { fprintf(stderr, "cp: %s: no such file\n", srcpath); return 1; }

    cop_inode_t src_nd;
    if (!inode_read(src_inode, &src_nd) || src_nd.type != 1) { fprintf(stderr, "cp: %s: not a file\n", srcpath); return 1; }

    uint8_t *buf; uint64_t size;
    if (!file_read_all(src_inode, &buf, &size)) { fprintf(stderr, "cp: read failed\n"); return 1; }

    uint64_t dst_inode;
    bool exists = fs_lookup(dstpath, &dst_inode);
    if (exists && !force) { fprintf(stderr, "cp: %s exists, use -f to overwrite\n", dstpath); free(buf); return 1; }

    if (!exists) {
        if (!fs_create(dstpath, 1)) { fprintf(stderr, "cp: create failed\n"); free(buf); return 1; }
        fs_lookup(dstpath, &dst_inode);
    }

    bool ok = file_write(dst_inode, buf, size);
    free(buf);
    if (!ok) { fprintf(stderr, "cp: write failed\n"); return 1; }
    return 0;
}

static int cmd_verify(int argc, char **argv) {
    static struct option opts[] = {
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };

    int c;
    optind = 0;
    opterr = 0;

    while ((c = getopt_long(argc, argv, "h", opts, NULL)) != -1) {
        printf("usage: verify [path]\n");
        return 0;
    }

    char path[COP_MAX_PATH];

    const char *arg = optind < argc ? argv[optind] : NULL;

    if (!resolve_path(arg, path, sizeof(path))) {
        printf("0\n");
        return 0;
    }

    uint64_t inode;

    if (fs_lookup(path, &inode))
        printf("1\n");
    else
        printf("0\n");

    return 0;
}

// =================================================================
// Command: help
// =================================================================
static void print_help(void) {
    printf("Tip: run '%s <image> --help' from your shell (not in here) for\n", "copsh");
    printf("one-shot flags like -g/-p/-m that run a single command and exit.\n\n");
    printf("Navigation:\n");
    printf("  ls [-l|--long] [-a|--all] [-h|--help] [path]\n");
    printf("                         list current directory\n");
    printf("  cd [-h|--help] <name|..|/>\n");
    printf("                         change directory\n");
    printf("  pwd [-h|--help]        print current path\n");
    printf("  tree [-d|--depth N] [-h|--help] [path]\n");
    printf("                         recursive listing from current directory\n");
    printf("  find [-i|--ignore-case] [-t|--type f|d] [-h|--help] <name>\n");
    printf("                         search entire filesystem for a name\n");
    printf("Reading files:\n");
    printf("  cat [-n|--number] [-h|--help] <name>\n");
    printf("                         print file contents\n");
    printf("  head [-n|--lines N] [-c|--bytes N] [-h|--help] <name> [n]\n");
    printf("                         print first n lines (default 10)\n");
    printf("  hexdump [-n|--length N] [-o|--offset N] [-h|--help] <name>\n");
    printf("                         hex + ascii dump of file contents\n");
    printf("  stat [-h|--help] <name>\n");
    printf("                         show inode metadata\n");
    printf("  df [-h|--human] [--help]\n");
    printf("                         show block/inode usage\n");
    printf("Host <-> image transfer:\n");
    printf("  get [-f|--force] [-h|--help] <name> <hostpath>\n");
    printf("                         copy a file out of the image\n");
    printf("  put [-f|--force] [-h|--help] <hostpath> <name>\n");
    printf("                         copy a file into the current directory\n");
    printf("Modifying the image:\n");
    printf("  mkdir [-p|--parents] [-h|--help] <name>\n");
    printf("                         create a directory\n");
    printf("  touch [-h|--help] <name>\n");
    printf("                         create an empty file / update timestamp\n");
    printf("  rm [-f|--force] [-h|--help] <name>\n");
    printf("                         delete a file\n");
    printf("  rmdir [-f|--force] [-h|--help] <name>\n");
    printf("                         delete an empty directory (-f forces recursive)\n");
    printf("  mv [-f|--force] [-h|--help] <old> <new>\n");
    printf("                         rename an entry (same directory only)\n");
    printf("  cp [-f|--force] [-h|--help] <name> <newname>\n");
    printf("                         copy a file (any path within the image)\n");
    printf("Other:\n");
    printf("  verify  <name>         print 1 or the path exsist and 0 if dont ");
    printf("  help                   show this message\n");
    printf("  exit / quit            leave the shell\n");
}

// =================================================================
// One-shot CLI mode (like ata.run) - run a single command against
// the image and exit, instead of dropping into the interactive
// shell. Everything after the image path is parsed as flags.
// =================================================================
static void print_cli_usage(const char *prog) {
    printf("usage: %s <disk-image>                    interactive shell\n", prog);
    printf("       %s <disk-image> [flags]             one-shot command\n\n", prog);
    printf("One-shot flags (run one command against the image, then exit):\n");
    printf("  -g, --get <name> <hostpath>   copy a file out of the image\n");
    printf("  -p, --put <hostpath> <name>   copy a file into the image\n");
    printf("  -m, --rmdir <name>            remove a directory\n");
    printf("  -l, --list [path]             list a directory (default: /)\n");
    printf("  -c, --cat <name>              print a file's contents\n");
    printf("  -k, --mkdir <name>            create a directory\n");
    printf("  -t, --touch <name>            create an empty file / update timestamp\n");
    printf("  -r, --rm <name>               delete a file\n");
    printf("  -v, --verify <path>          check if path exists (prints 1/0)\n");
    printf("  -f, --force                   modifier for -g/-p/-r/-m:\n");
    printf("                                   -g/-p: overwrite existing destination\n");
    printf("                                   -r:    ignore a missing file\n");
    printf("                                   -m:    remove non-empty directories too\n");
    printf("                                   -k:    also create missing parent dirs\n");
    printf("  -h, --help                    show this message\n");
}

static struct option g_cli_opts[] = {
    {"get",   no_argument, 0, 'g'},
    {"put",   no_argument, 0, 'p'},
    {"rmdir", no_argument, 0, 'm'},
    {"list",  no_argument, 0, 'l'},
    {"cat",   no_argument, 0, 'c'},
    {"mkdir", no_argument, 0, 'k'},
    {"touch", no_argument, 0, 't'},
    {"rm",    no_argument, 0, 'r'},
    {"force", no_argument, 0, 'f'},
    {"help",  no_argument, 0, 'h'},
    {"verify", no_argument, 0, 'v'},
    {0,0,0,0}
};

typedef enum {
    ACT_NONE, ACT_GET, ACT_PUT, ACT_RMDIR, ACT_LIST,
    ACT_CAT, ACT_MKDIR, ACT_TOUCH, ACT_RM, ACT_VERIFY
} cli_action_t;

// Returns true if a one-shot action was taken (caller should exit
// with the returned code); false means "fall through to the
// interactive shell" (no action flags were given).
static bool run_cli_action(const char *prog, int argc, char **argv, int *exit_code) {
    // Build a sub-argv skipping argv[1] (the disk image path) so
    // getopt_long sees a clean argc/argv pair starting at index 0.
    int subargc = argc - 1;
    char **subargv = malloc(sizeof(char *) * (size_t)subargc);
    subargv[0] = argv[0];
    for (int i = 2; i < argc; i++) subargv[i - 1] = argv[i];

    cli_action_t action = ACT_NONE;
    bool force = false;
    int c;
    optind = 1;
    opterr = 0;

    while ((c = getopt_long(subargc, subargv, "gpmlcktrfhv", g_cli_opts, NULL)) != -1) {
        switch (c) {
            case 'g': action = ACT_GET; break;
            case 'p': action = ACT_PUT; break;
            case 'm': action = ACT_RMDIR; break;
            case 'l': action = ACT_LIST; break;
            case 'c': action = ACT_CAT; break;
            case 'k': action = ACT_MKDIR; break;
            case 't': action = ACT_TOUCH; break;
            case 'r': action = ACT_RM; break;
            case 'v': action = ACT_VERIFY; break;
            case 'f': force = true; break;
            case 'h':
                print_cli_usage(prog);
                free(subargv);
                *exit_code = 0;
                return true;
            default:
                print_cli_usage(prog);
                free(subargv);
                *exit_code = 1;
                return true;
        }
    }

    if (action == ACT_NONE) {
        free(subargv);
        return false; // no action flags at all -> interactive shell
    }

    int rc = 0;
    int cargc = 0;
    char *cargv[8];

    switch (action) {
        case ACT_GET:
            if (optind + 1 >= subargc) {
                fprintf(stderr, "%s: -g/--get requires <name> <hostpath>\n", prog);
                rc = 1; break;
            }
            cargv[cargc++] = "get";
            if (force) cargv[cargc++] = "-f";
            cargv[cargc++] = subargv[optind];
            cargv[cargc++] = subargv[optind + 1];
            rc = cmd_get(cargc, cargv);
            break;

        case ACT_PUT:
            if (optind + 1 >= subargc) {
                fprintf(stderr, "%s: -p/--put requires <hostpath> <name>\n", prog);
                rc = 1; break;
            }
            cargv[cargc++] = "put";
            if (force) cargv[cargc++] = "-f";
            cargv[cargc++] = subargv[optind];
            cargv[cargc++] = subargv[optind + 1];
            rc = cmd_put(cargc, cargv);
            break;

        case ACT_RMDIR:
            if (optind >= subargc) {
                fprintf(stderr, "%s: -m/--rmdir requires <name>\n", prog);
                rc = 1; break;
            }
            cargv[cargc++] = "rmdir";
            if (force) cargv[cargc++] = "-f";
            cargv[cargc++] = subargv[optind];
            rc = cmd_rmdir(cargc, cargv);
            break;

        case ACT_LIST:
            cargv[cargc++] = "ls";
            cargv[cargc++] = "-l";
            if (optind < subargc) cargv[cargc++] = subargv[optind];
            rc = cmd_ls(cargc, cargv);
            break;

        case ACT_CAT:
            if (optind >= subargc) {
                fprintf(stderr, "%s: -c/--cat requires <name>\n", prog);
                rc = 1; break;
            }
            cargv[cargc++] = "cat";
            cargv[cargc++] = subargv[optind];
            rc = cmd_cat(cargc, cargv);
            break;

        case ACT_MKDIR:
            if (optind >= subargc) {
                fprintf(stderr, "%s: -k/--mkdir requires <name>\n", prog);
                rc = 1; break;
            }
            cargv[cargc++] = "mkdir";
            if (force) cargv[cargc++] = "-p"; // -f = also create missing parents
            cargv[cargc++] = subargv[optind];
            rc = cmd_mkdir(cargc, cargv);
            break;

        case ACT_TOUCH:
            if (optind >= subargc) {
                fprintf(stderr, "%s: -t/--touch requires <name>\n", prog);
                rc = 1; break;
            }
            cargv[cargc++] = "touch";
            cargv[cargc++] = subargv[optind];
            rc = cmd_touch(cargc, cargv);
            break;

        case ACT_RM:
            if (optind >= subargc) {
                fprintf(stderr, "%s: -r/--rm requires <name>\n", prog);
                rc = 1; break;
            }
            cargv[cargc++] = "rm";
            if (force) cargv[cargc++] = "-f";
            cargv[cargc++] = subargv[optind];
            rc = cmd_rm(cargc, cargv);
            break;

        case ACT_VERIFY:
        if (optind >= subargc) {
            fprintf(stderr, "%s: --verify requires <path>\n", prog);
            rc = 1;
            break;
        }

        cargv[cargc++] = "verify";
        cargv[cargc++] = subargv[optind];

        rc = cmd_verify(cargc, cargv);
        break;

        case ACT_NONE:
        default:
            break;
    }

    free(subargv);
    *exit_code = rc;
    return true;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <disk-image> [flags]\n", argv[0]);
        fprintf(stderr, "       %s <disk-image> --help  for one-shot flags\n", argv[0]);
        return 1;
    }

    g_disk = fopen(argv[1], "r+b");
    if (!g_disk) {
        fprintf(stderr, "copsh: cannot open %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    if (!sb_load()) {
        fclose(g_disk);
        return 1;
    }

    g_cwd_inode = g_sb.root_inode;
    strcpy(g_cwd_path, "/");

    if (argc > 2) {
        int exit_code = 0;
        if (run_cli_action(argv[0], argc, argv, &exit_code)) {
            fclose(g_disk);
            return exit_code;
        }
        // no action flags recognized in the extra args -> fall through
    }

    printf("copsh: mounted %s (COPFS v0x%08x, %llu/%llu blocks free, %llu/%llu inodes free)\n",
           argv[1], g_sb.version,
           (unsigned long long)g_sb.free_blocks, (unsigned long long)(g_sb.total_blocks - g_sb.data_start),
           (unsigned long long)g_sb.free_inodes, (unsigned long long)g_sb.total_inodes);
    printf("type 'help' for commands\n");

    char line[4096];
    char *cargv[64];

    while (1) {
        printf("cop:%s$ ", g_cwd_path);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) { printf("\n"); break; }
        line[strcspn(line, "\n")] = '\0';

        char linecopy[4096];
        strncpy(linecopy, line, sizeof(linecopy) - 1);
        linecopy[sizeof(linecopy) - 1] = '\0';

        int cargc = tokenize(linecopy, cargv, 64);
        if (cargc == 0) continue;

        const char *cmd = cargv[0];

        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) break;
        else if (strcmp(cmd, "help") == 0) print_help();
        else if (strcmp(cmd, "ls") == 0) cmd_ls(cargc, cargv);
        else if (strcmp(cmd, "cd") == 0) cmd_cd(cargc, cargv);
        else if (strcmp(cmd, "pwd") == 0) cmd_pwd(cargc, cargv);
        else if (strcmp(cmd, "tree") == 0) cmd_tree(cargc, cargv);
        else if (strcmp(cmd, "find") == 0) cmd_find(cargc, cargv);
        else if (strcmp(cmd, "cat") == 0) cmd_cat(cargc, cargv);
        else if (strcmp(cmd, "head") == 0) cmd_head(cargc, cargv);
        else if (strcmp(cmd, "hexdump") == 0) cmd_hexdump(cargc, cargv);
        else if (strcmp(cmd, "stat") == 0) cmd_stat(cargc, cargv);
        else if (strcmp(cmd, "df") == 0) cmd_df(cargc, cargv);
        else if (strcmp(cmd, "get") == 0) cmd_get(cargc, cargv);
        else if (strcmp(cmd, "put") == 0) cmd_put(cargc, cargv);
        else if (strcmp(cmd, "mkdir") == 0) cmd_mkdir(cargc, cargv);
        else if (strcmp(cmd, "touch") == 0) cmd_touch(cargc, cargv);
        else if (strcmp(cmd, "rm") == 0) cmd_rm(cargc, cargv);
        else if (strcmp(cmd, "rmdir") == 0) cmd_rmdir(cargc, cargv);
        else if (strcmp(cmd, "mv") == 0) cmd_mv(cargc, cargv);
        else if (strcmp(cmd, "cp") == 0) cmd_cp(cargc, cargv);
        else if (strcmp(cmd, "verify") == 0) cmd_verify(cargc, cargv);
        else fprintf(stderr, "copsh: unknown command '%s' (try 'help')\n", cmd);
    }

    (void)g_dirty_prompt_shown;
    fclose(g_disk);
    return 0;
}
