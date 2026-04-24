#pragma once

#include <core/efi.h>

typedef struct {
    UINTN                 size;
    EFI_MEMORY_DESCRIPTOR *map;
    UINTN                 key;
    UINTN                 desc_size;
    UINT32                desc_version;
} Memory_Map_Info;

// Bitmapped font info (assuming monospaced)
typedef struct {
    char     *name;             // Font name
    uint32_t width;             // Glyph width in pixels
    uint32_t height;            // Glyph height in pixels
    uint32_t num_glyphs;        // Number of glyphs in array/font
    uint8_t  *glyphs;           // Glyph data/array
    bool     left_col_first;    // Are bits for glyphs stored in memory left->right 
                                //   e.g. PSF font, or right->left e.g. terminus?
} Bitmap_Font;

typedef struct {
    Memory_Map_Info                   mmap; 
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE gop_mode;
    EFI_RUNTIME_SERVICES              *RuntimeServices;
    UINTN                             NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE           *ConfigurationTable;
    UINTN                             num_fonts;
    Bitmap_Font                       *fonts;
} Kernel_Parms;

typedef void EFIAPI (*Entry_Point)(Kernel_Parms *);