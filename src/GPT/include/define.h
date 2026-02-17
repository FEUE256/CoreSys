#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <uchar.h> 
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

// -------------------------------------
// Global Typedefs
// -------------------------------------
// Globally Unique IDentifier (aka UUID) 
typedef struct {
    uint32_t time_lo;
    uint16_t time_mid;
    uint16_t time_hi_and_ver;       // Highest 4 bits are version #
    uint8_t clock_seq_hi_and_res;   // Highest bits are variant #
    uint8_t clock_seq_lo;
    uint8_t node[6];
} __attribute__ ((packed)) Guid;

// MBR Partition
typedef struct {
    uint8_t boot_indicator;
    uint8_t starting_chs[3];
    uint8_t os_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t size_lba;
} __attribute__ ((packed)) Mbr_Partition;

// Master Boot Record
typedef struct {
    uint8_t boot_code[440];
    uint32_t mbr_signature;
    uint16_t unknown;
    Mbr_Partition partition[4];
    uint16_t boot_signature;
} __attribute__ ((packed)) Mbr;

// GPT Header
typedef struct {
    uint8_t signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved_1;
    uint64_t my_lba;
    uint64_t alternate_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    Guid disk_guid;
    uint64_t partition_table_lba;
    uint32_t number_of_entries;
    uint32_t size_of_entry;
    uint32_t partition_table_crc32;

    uint8_t reserved_2[512-92];
} __attribute__ ((packed)) Gpt_Header;

// GPT Partition Entry
typedef struct {
    Guid partition_type_guid;
    Guid unique_guid;
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    char16_t name[36];  // UCS-2 (UTF-16 limited to code points 0x0000 - 0xFFFF)
} __attribute__ ((packed)) Gpt_Partition_Entry;

// FAT32 Volume Boot Record (VBR)
typedef struct {
    uint8_t  BS_jmpBoot[3];
    uint8_t  BS_OEMName[8];
    uint16_t BPB_BytesPerSec;
    uint8_t  BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t  BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t  BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t  BPB_Reserved[12];
    uint8_t  BS_DrvNum;
    uint8_t  BS_Reserved1;
    uint8_t  BS_BootSig;
    uint8_t  BS_VolID[4];
    uint8_t  BS_VolLab[11];
    uint8_t  BS_FilSysType[8];

    // Not in fatgen103.doc tables
    uint8_t  boot_code[510-90];
    uint16_t bootsect_sig;      // 0xAA55
} __attribute__ ((packed)) Vbr;

// FAT32 File System Info Sector
typedef struct {
    uint32_t FSI_LeadSig;
    uint8_t  FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
    uint32_t FSI_Nxt_Free;
    uint8_t  FSI_Reserved2[12];
    uint32_t FSI_TrailSig;
} __attribute__ ((packed)) FSInfo;

// FAT32 Directory Entry (Short Name)
typedef struct {
    uint8_t  DIR_Name[11];
    uint8_t  DIR_Attr;
    uint8_t  DIR_NTRes;
    uint8_t  DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} __attribute__ ((packed)) FAT32_Dir_Entry_Short;

// FAT32 Directory Entry Attributes
typedef enum {
    ATTR_READ_ONLY = 0x01,
    ATTR_HIDDEN    = 0x02,
    ATTR_SYSTEM    = 0x04,
    ATTR_VOLUME_ID = 0x08,
    ATTR_DIRECTORY = 0x10,
    ATTR_ARCHIVE   = 0x20,
    ATTR_LONG_NAME = ATTR_READ_ONLY | ATTR_HIDDEN |
                     ATTR_SYSTEM    | ATTR_VOLUME_ID,
} FAT32_Dir_Attr;


// FAT32 File "types"
typedef enum {
    TYPE_DIR,   // Directory
    TYPE_FILE,  // Regular file
} File_Type;

// Common Virtual Hard Disk Footer, for a "fixed" vhd
// All fields are in network byte order (Big Endian),
//   since I'm lazy or otherwise a bad programmer,
//   we'll use byte arrays here
typedef struct {
    uint8_t cookie[8];
    uint8_t features[4];
    uint8_t version[4];
    uint64_t data_offset;
    uint8_t timestamp[4];
    uint8_t creator_app[4];
    uint8_t creator_ver[4];
    uint8_t creator_OS[4];
    uint8_t original_size[8];
    uint8_t current_size[8];
    uint8_t disk_geometry[4];
    uint8_t disk_type[4];
    uint8_t checksum[4];
    Guid unique_id;
    uint8_t saved_state;
    uint8_t reserved[427];
} __attribute__ ((packed)) Vhd;

// Internal Options object for commandline args
typedef struct {
    char *image_name;
    uint32_t lba_size;
    uint32_t esp_size;
    uint32_t data_size;
    char **esp_file_paths;
    uint32_t num_esp_file_paths;
    FILE **esp_files;
    char **data_files;
    char *efi_file;
    uint32_t num_data_files;
    bool vhd;
    bool help;
    bool error;
} Options;

// -------------------------------------
// Global constants, enums
// -------------------------------------
// EFI System Partition GUID
const Guid ESP_GUID = { 0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, 
                        { 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B } };

// (Microsoft) Basic Data GUID
const Guid BASIC_DATA_GUID = { 0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0,
                                { 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 } };

enum {
    GPT_TABLE_ENTRY_SIZE = 128,
    NUMBER_OF_GPT_TABLE_ENTRIES = 128,
    GPT_TABLE_SIZE = 16384,             // Minimum size per UEFI spec 2.10
    ALIGNMENT = 1048576,                // 1 MiB alignment value
};

// -------------------------------------
// Global Variables
// -------------------------------------
uint64_t lba_size = 512;
uint64_t esp_size = 1024*1024*33;   // 33 MiB
uint64_t data_size = 1024*1024*1;   // 1 MiB
uint64_t image_size = 0;
uint64_t esp_size_lbas = 0, data_size_lbas = 0, image_size_lbas = 0,  
         gpt_table_lbas = 0;                              // Sizes in lbas
uint64_t align_lba = 0, esp_lba = 0, data_lba = 0,
         fat32_fats_lba = 0, fat32_data_lba = 0;          // Starting LBA values

bool opened_info_file = false;
