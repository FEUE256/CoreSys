#pragma once

#include <core/efi.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    // Memory map info
    UINTN MemoryMapSize;               // Size of the memory map in bytes
    UINTN DescriptorSize;              // Size of each memory descriptor
    UINT32 DescriptorVersion;          // Version of memory descriptor
    EFI_MEMORY_DESCRIPTOR* MemoryMap; // Pointer to EFI memory map

    // Framebuffer info
    VOID* FramebufferBase;             // Base address of the framebuffer
    UINT32 FramebufferWidth;           // Width in pixels
    UINT32 FramebufferHeight;          // Height in pixels
    UINT32 FramebufferBPP;             // Bits per pixel
    UINT32 FramebufferPitch;           // Bytes per scanline

    // ACPI info
    VOID* Rsdp;                        // Root System Description Pointer

    // Command line
    CHAR16* CmdLine;                   // UTF-16 command line string

    // Additional user data area
    int DAUDA;                        // Pointer to extra user-defined data
    int safe;

    EFI_SYSTEM_TABLE *SystemTable;        // Pointer to the EFI system table for kernel use
} kargs;
