#pragma once

static EFI_GUID gEfiAcpi20TableGuid =
{0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, {0x00,0x80,0xc7,0x3c,0x88,0x81}};

static EFI_GUID gEfiAcpi10TableGuid =
{0xeb9d2d30, 0x2d88, 0x11d3, 0x9a, 0x16, {0x00,0x90,0x27,0x3f,0xc1,0x4d}};
    
typedef struct
{
    char Signature[8];

    uint8_t Checksum;

    char OEMID[6];

    uint8_t Revision;

    uint32_t RsdtAddress;


    uint32_t Length;

    uint64_t XsdtAddress;

    uint8_t ExtendedChecksum;

    uint8_t Reserved[3];

} ACPI_RSDP;
