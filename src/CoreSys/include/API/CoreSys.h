#pragma once

#include <core/efi.h>         // Basic UEFI types
#include <core/stdio_efi.h>   // Custom printf for UEFI
#include <stdarg.h>
#include <init/funcs.h>

#define CORESYS_H_INCLUDED

bool printf_c16(const CHAR16 *fmt, ...);
void sf(char *file, int line, const char *func,
        EFI_STATUS status, const CHAR16 *fmt, ...);

int fw_safe_exec_int = 0;

#include <stdint.h>

typedef struct EFI_HEADER {
    uint16_t magic;          // Signature (e.g. 'UE' or custom)
    uint8_t  version_major;
    uint8_t  version_minor;

    uint32_t header_size;    // Size of this header struct
    uint32_t checksum;       // Optional integrity checksum

    uint64_t entry_point;    // Entry address (EFI image entry)
    uint64_t image_base;     // Loaded base address in memory

    uint64_t size_of_image;  // Total image size in bytes

    uint32_t flags;          // Feature flags (boot mode, secure boot, etc.)
    uint32_t reserved;

    char     signature[8];   // Human-readable identifier (e.g. "CoreSys")
} EFI_HEADER;

typedef struct RE_HEADER {
    char magic[2]; // NE
    EFI_HEADER efi_header; // EFI HEADER
} RE_HEADER;

void fw_safe_exec(void) {
    fw_safe_exec_int = 1;
}

void fw_unsafe_exec(void) {
    fw_safe_exec_int = 0;
}

typedef VOID *CS_HANDLE;

typedef enum CS_STATUS {
    CS_STATUS_OK = 0,
    CS_STATUS_ERROR = 1,

    CS_STATUS_UNKNOWN = 2,
    CS_STATUS_INVALID_ARGUMENT = 3,
    CS_STATUS_NULL_POINTER = 4,
    CS_STATUS_OUT_OF_MEMORY = 5,
    CS_STATUS_BUFFER_OVERFLOW = 6,
    CS_STATUS_BUFFER_UNDERFLOW = 7,
    CS_STATUS_TIMEOUT = 8,
    CS_STATUS_NOT_FOUND = 9,

    CS_STATUS_ALREADY_EXISTS = 10,
    CS_STATUS_PERMISSION_DENIED = 11,
    CS_STATUS_ACCESS_VIOLATION = 12,
    CS_STATUS_NOT_IMPLEMENTED = 13,
    CS_STATUS_UNSUPPORTED = 14,
    CS_STATUS_DEPENDENCY_FAILURE = 15,
    CS_STATUS_INITIALIZATION_FAILED = 16,
    CS_STATUS_SHUTDOWN_FAILED = 17,
    CS_STATUS_IO_ERROR = 18,
    CS_STATUS_DISK_FULL = 19,

    CS_STATUS_FILE_NOT_FOUND = 20,
    CS_STATUS_FILE_CORRUPTED = 21,
    CS_STATUS_FILE_READ_ERROR = 22,
    CS_STATUS_FILE_WRITE_ERROR = 23,
    CS_STATUS_PATH_TOO_LONG = 24,
    CS_STATUS_DIRECTORY_NOT_EMPTY = 25,
    CS_STATUS_DIRECTORY_NOT_FOUND = 26,
    CS_STATUS_INVALID_PATH = 27,
    CS_STATUS_INVALID_FORMAT = 28,
    CS_STATUS_PARSE_ERROR = 29,

    CS_STATUS_NETWORK_UNAVAILABLE = 30,
    CS_STATUS_CONNECTION_FAILED = 31,
    CS_STATUS_CONNECTION_LOST = 32,
    CS_STATUS_PROTOCOL_ERROR = 33,
    CS_STATUS_HOST_UNREACHABLE = 34,
    CS_STATUS_PORT_UNAVAILABLE = 35,
    CS_STATUS_DNS_FAILURE = 36,
    CS_STATUS_NETWORK_TIMEOUT = 37,
    CS_STATUS_PACKET_LOSS = 38,
    CS_STATUS_SOCKET_ERROR = 39,

    CS_STATUS_THREAD_ERROR = 40,
    CS_STATUS_MUTEX_ERROR = 41,
    CS_STATUS_DEADLOCK_DETECTED = 42,
    CS_STATUS_SYNCHRONIZATION_FAILED = 43,
    CS_STATUS_TASK_FAILED = 44,
    CS_STATUS_SCHEDULER_ERROR = 45,
    CS_STATUS_CONTEXT_SWITCH_FAILED = 46,
    CS_STATUS_INTERRUPT_ERROR = 47,
    CS_STATUS_TIMER_ERROR = 48,
    CS_STATUS_CPU_LIMIT_REACHED = 49,

    CS_STATUS_SECURITY_VIOLATION = 50,
    CS_STATUS_AUTH_FAILED = 51,
    CS_STATUS_AUTH_REQUIRED = 52,
    CS_STATUS_TOKEN_INVALID = 53,
    CS_STATUS_TOKEN_EXPIRED = 54,
    CS_STATUS_ENCRYPTION_FAILED = 55,
    CS_STATUS_DECRYPTION_FAILED = 56,
    CS_STATUS_SIGNATURE_INVALID = 57,
    CS_STATUS_CHECKSUM_MISMATCH = 58,
    CS_STATUS_TAMPER_DETECTED = 59,

    CS_STATUS_DEVICE_NOT_READY = 60,
    CS_STATUS_DEVICE_BUSY = 61,
    CS_STATUS_DEVICE_ERROR = 62,
    CS_STATUS_DRIVER_MISSING = 63,
    CS_STATUS_DRIVER_FAILURE = 64,
    CS_STATUS_HARDWARE_FAILURE = 65,
    CS_STATUS_UNSUPPORTED_DEVICE = 66,
    CS_STATUS_DEVICE_DISCONNECTED = 67,
    CS_STATUS_RESOURCE_CONFLICT = 68,
    CS_STATUS_POWER_FAILURE = 69,

    CS_STATUS_CONFIGURATION_ERROR = 70,
    CS_STATUS_CONFIG_MISSING = 71,
    CS_STATUS_CONFIG_INVALID = 72,
    CS_STATUS_CONFIG_LOCKED = 73,
    CS_STATUS_ENVIRONMENT_ERROR = 74,
    CS_STATUS_VERSION_MISMATCH = 75,
    CS_STATUS_BUILD_FAILURE = 76,
    CS_STATUS_COMPILE_ERROR = 77,
    CS_STATUS_LINKER_ERROR = 78,
    CS_STATUS_RUNTIME_ERROR = 79,

    CS_STATUS_OVERFLOW = 80,
    CS_STATUS_UNDERFLOW = 81,
    CS_STATUS_DIVIDE_BY_ZERO = 82,
    CS_STATUS_FLOATING_POINT_ERROR = 83,
    CS_STATUS_ASSERTION_FAILED = 84,
    CS_STATUS_PRECONDITION_FAILED = 85,
    CS_STATUS_POSTCONDITION_FAILED = 86,
    CS_STATUS_STATE_CORRUPTED = 87,
    CS_STATUS_DATA_LOSS = 88,
    CS_STATUS_RECOVERY_FAILED = 89,

    CS_STATUS_BACKUP_FAILED = 90,
    CS_STATUS_RESTORE_FAILED = 91,
    CS_STATUS_SYNC_FAILED = 92,
    CS_STATUS_QUOTA_EXCEEDED = 93,
    CS_STATUS_RATE_LIMITED = 94,
    CS_STATUS_THROTTLED = 95,
    CS_STATUS_SERVICE_UNAVAILABLE = 96,
    CS_STATUS_MAINTENANCE_MODE = 97,
    CS_STATUS_FEATURE_DISABLED = 98,
    CS_STATUS_FATAL_ERROR = 99
} cs_status;

#define CS_SUCCESS 0
#define CS_ERROR   1

void CopyMem(void* dst, const void* src, UINTN size)
{
    UINT8* d = dst;
    const UINT8* s = src;
    for (UINTN i = 0; i < size; i++)
        d[i] = s[i];
}

typedef enum {
    CS_POOL_LOADER = EfiLoaderData
} cs_memory_type;

/* malloc */
cs_status cs_malloc(
    UINTN size,
    void** outPtr
) {
    if (!gST || !outPtr || size == 0) {
        return CS_STATUS_INVALID_ARGUMENT;
    }

    *outPtr = NULL;

    EFI_STATUS status = gST->BootServices->AllocatePool(
        (EFI_MEMORY_TYPE)CS_POOL_LOADER,
        size,
        outPtr
    );

    if (EFI_ERROR(status)) {
        return CS_STATUS_OUT_OF_MEMORY;
    }

    return CS_STATUS_OK;
}

/* realloc-style */
void* cs_mlock(void* oldPtr, UINTN oldSize, UINTN newSize)
{
    void* newPtr = NULL;

    EFI_STATUS status = gBS->AllocatePool(EfiLoaderData, newSize, &newPtr);
    if (EFI_ERROR(status))
        return NULL;

    if (oldPtr) {
        CopyMem(newPtr, oldPtr, oldSize);
        gBS->FreePool(oldPtr);
    }

    return newPtr;
}

/* free */
cs_status cs_mfree(
    void* ptr
) {
    if (!gST) {
        return CS_STATUS_INVALID_ARGUMENT;
    }

    if (!ptr) {
        return CS_STATUS_OK;
    }

    EFI_STATUS status = gST->BootServices->FreePool(ptr);

    if (EFI_ERROR(status)) {
        return CS_STATUS_ERROR;
    }

    return CS_STATUS_OK;
}

void cs_sleep(uint64_t ms) {
    if (ms == 0) return;

    /*
        Stall() takes microseconds, not milliseconds.
        1 ms = 1000 µs
    */

    UINTN microseconds;

    while (ms > 0) {

        /*
            Prevent overflow for large values:
            handle in chunks (e.g., 1000 ms = 1,000,000 µs)
        */

        uint64_t chunk = (ms > 1000) ? 1000 : ms;

        microseconds = (UINTN)(chunk * 1000ULL);

        gBS->Stall(microseconds);

        ms -= chunk;
    }
}

typedef enum {
    CS_LOG_SF,
    CS_LOG_ALERT,
    CS_LOG_WARNING,
    CS_LOG_INFO,
    CS_LOG_DEBUG
} cs_log_level;

static cs_log_level g_log_level = CS_LOG_INFO;

void cs_log_set_level(cs_log_level level)
{
    g_log_level = level;
}

void cs_log(cs_log_level level, const CHAR16* msg)
{
    if (level > g_log_level)
        return;

    switch (level)
    {
        case CS_LOG_SF:
            sf(__FILE__, __LINE__, __func__, EFI_SUCCESS, msg);
            break;

        case CS_LOG_ALERT:
            error(EFI_SUCCESS, u"%s\r\n", msg);
            break;

        case CS_LOG_WARNING:
            printf_c16(u"WARNING: ");
            printf_c16(msg);
            printf_c16(u"\r\n");
            break;

        case CS_LOG_INFO:
            printf_c16(u"INFO: ");
            printf_c16(msg);
            printf_c16(u"\r\n");
            break;

        case CS_LOG_DEBUG:
            printf_c16(u"DEBUG: ");
            printf_c16(msg);
            printf_c16(u"\r\n");
            break;

        default:
            printf_c16(u"UNKNOWN LOG LEVEL\r\n");
            printf_c16(msg);
            printf_c16(u"\r\n");
            break;
    }
}


bool cs_logf(cs_log_level level, const CHAR16* fmt, ...)
{
    CHAR16 buffer[512];

    va_list args;
    va_start(args, fmt);

    // Format into buffer (must exist in your system)
    bool result = vswprintf(buffer, 512, fmt, args);

    va_end(args);

    // Send formatted string into your log router
    cs_log(level, buffer);

    return result;
}

/* Function entry type */
typedef void (*cs_entry_t)(void);

/* CoreSys file representation */
typedef struct {
    void *data;        // raw payload (non-executable data only)
    size_t size;       // payload size
    cs_entry_t entry;  // executable entry point (if any)
} cs_file;

/* Execute file */
void cs_task_run_file(cs_file *file)
{
    if (!file)
        return;

    if (file->entry)
        file->entry();
}
