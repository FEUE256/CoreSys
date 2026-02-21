/** @file
  Definitions used in both PEI and DXE phases of UEFI.

Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __UEFI_MULTIPHASE_H__
#define __UEFI_MULTIPHASE_H__

///
/// Attributes of variable.
///
#define EFI_VARIABLE_NON_VOLATILE        0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS  0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS      0x00000004
///
/// Hardware error record attribute.
///
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD  0x00000008
///
/// Authenticated variable attributes.
///
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS  0x00000020
#define EFI_VARIABLE_APPEND_WRITE                           0x00000040
///
/// Deprecated attribute
///
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS  0x00000010

#ifndef VFRCOMPILE
  #include <Guid/WinCertificate.h>

  ///
  /// Memory types in UEFI (typedef + #define)
  ///
  typedef unsigned int EFI_MEMORY_TYPE;

  // Standard memory types
  #define EfiReservedMemoryType          0
  #define EfiLoaderCode                  1
  #define EfiLoaderData                  2
  #define EfiBootServicesCode            3
  #define EfiBootServicesData            4
  #define EfiRuntimeServicesCode         5
  #define EfiRuntimeServicesData         6
  #define EfiConventionalMemory          7
  #define EfiUnusableMemory              8
  #define EfiACPIReclaimMemory           9
  #define EfiACPIMemoryNVS               10
  #define EfiMemoryMappedIO              11
  #define EfiMemoryMappedIOPortSpace     12
  #define EfiPalCode                     13
  #define EfiPersistentMemory            14
  #define EfiUnacceptedMemoryType        15
  #define EfiMaxMemoryType               16

  // Reserved memory ranges
  #define MEMORY_TYPE_OEM_RESERVED_MIN   0x70000000U
  #define MEMORY_TYPE_OEM_RESERVED_MAX   0x7FFFFFFFU
  #define MEMORY_TYPE_OS_RESERVED_MIN    0x80000000U
  #define MEMORY_TYPE_OS_RESERVED_MAX    0xFFFFFFFFU

///
/// Enumeration of system reset types.
///
typedef enum {
  EfiResetCold,
  EfiResetWarm,
  EfiResetShutdown,
  EfiResetPlatformSpecific
} EFI_RESET_TYPE;

///
/// Standard EFI table header.
///
typedef struct {
  UINT64    Signature;
  UINT32    Revision;
  UINT32    HeaderSize;
  UINT32    CRC32;
  UINT32    Reserved;
} EFI_TABLE_HEADER;

///
/// Authentication info for EFI variables.
///
typedef struct {
  UINT64                       MonotonicCount;
  WIN_CERTIFICATE_UEFI_GUID    AuthInfo;
} EFI_VARIABLE_AUTHENTICATION;

typedef struct {
  EFI_TIME                     TimeStamp;
  WIN_CERTIFICATE_UEFI_GUID    AuthInfo;
} EFI_VARIABLE_AUTHENTICATION_2;

#endif // VFRCOMPILE

#endif // __UEFI_MULTIPHASE_H__