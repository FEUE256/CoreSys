#pragma once

#include <core/efi.h>

#include <stdint.h>

#define KDI 0x256        // Mem Addr
// If changed update kernel/asm/main.s

#define KSP 0x270   // Mem Addr

#define KUE 0x300 // Mem Addr

#define MAX_MEMORY_ENTRIES 256

typedef uint64_t UINT64;
typedef uint32_t UINT32;
typedef uint8_t  UINT8;

typedef struct
{
    UINT64 base;
    UINT64 length;
    UINT32 type;

} MEMORY_ENTRY;

typedef struct
{
    MEMORY_ENTRY memory_map[MAX_MEMORY_ENTRIES];

    UINT64 memory_entries;


    UINT64 installed_ram;
    UINT64 reserved_ram;


    UINT64 physical_address_width;
    UINT64 max_supported_memory;

    uint64_t rsdp;
    uint64_t rsdt;
    uint64_t xsdt;

    uint64_t acpi_revision;

    uint8_t acpi_available;

    uint64_t framebuffer;
    uint64_t framebuffer_size;

    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t format;
} BOOT_INFO;

#define BOOT_INFO_PTR ((BOOT_INFO*)KSP)
