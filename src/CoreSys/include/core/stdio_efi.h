#pragma once

// Custom stdio-like functions for UEFI bootloader
// WARNING: Not a full-featured stdio.h, use only for bootloader

#include <core/efi.h>      // Basic UEFI types
#include <core/efi_lib.h>  // Simple lib

#include <stdarg.h>
#include <stdbool.h>

EFI_SYSTEM_TABLE *st;
EFI_BOOT_SERVICES *bs;
EFI_RUNTIME_SERVICES *rs;
EFI_HANDLE image;

EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;
EFI_RUNTIME_SERVICES *gRS;

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout;
EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *cin;

void clear_screen(void) {
    cout->ClearScreen(cout);
}

// =========================================================
// CHAR16 number conversion functions
// =========================================================

// Signed integer to CHAR16
CHAR16* itow(int64_t val, CHAR16* buf) {
    CHAR16 tmp[20];
    int i = 0, j;
    bool neg = val < 0;
    if (neg) val = -val;
    do {
        tmp[i++] = L'0' + (val % 10);
        val /= 10;
    } while (val != 0);
    if (neg) tmp[i++] = L'-';
    tmp[i] = L'\0';

    for (j = 0; j < i; j++) buf[j] = tmp[i - j - 1];
    buf[i] = L'\0';
    return buf;
}

// Unsigned integer to CHAR16
CHAR16* utow(uint64_t val, CHAR16* buf) {
    CHAR16 tmp[20];
    int i = 0, j;
    do {
        tmp[i++] = L'0' + (val % 10);
        val /= 10;
    } while (val != 0);
    tmp[i] = L'\0';

    for (j = 0; j < i; j++) buf[j] = tmp[i - j - 1];
    buf[i] = L'\0';
    return buf;
}

// Unsigned long long to CHAR16
CHAR16* ultow(unsigned long long val, CHAR16* buf) {
    return utow(val, buf);
}

#define MAX_EFI_ERROR 36
const CHAR16 *EFI_ERROR_STRINGS[MAX_EFI_ERROR] = {
    [3]  = u"0x0203000000000100000200010000000B0000000000000000000000000102",
    [5]  = u"0x0203000000000100000200010000000B0000000000000000000000000202",
    [7]  = u"0x0203000000000100000200010000000B0000000000000000000000000302",
    [14] = u"0x0203000000000100000200010000000B0000000000000000000000000402",
    [27] = u"0x0203000000000100000200010000000B0000000000000000000000000502",
};

// =========================================================
// Custom printf
// =========================================================
bool printf(const CHAR16 *format, ...)
{
    UINTN row = gST->ConOut->Mode->CursorRow;
    gST->ConOut->SetCursorPosition(gST->ConOut, 0, row);

    va_list args;
    va_start(args, format);

    CHAR16 buffer[1024];
    UINTN buf_index = 0;

    while (*format && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1)) {

        if (*format == L'%') {
            format++;

            // %% literal
            if (*format == L'%') {
                buffer[buf_index++] = L'%';
                format++;
                continue;
            }

            // %04x
            if (*format == L'0' && *(format + 1) == L'4' && *(format + 2) == L'x') {
                format += 3;

                unsigned int val = va_arg(args, unsigned int);

                CHAR16 tmp[5];
                tmp[4] = L'\0';

                static const CHAR16 hexchars[] = L"0123456789abcdef";

                for (int i = 3; i >= 0; i--) {
                    tmp[i] = hexchars[val & 0xF];
                    val >>= 4;
                }

                for (int i = 0; i < 4 && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1); i++) {
                    buffer[buf_index++] = tmp[i];
                }

                continue;
            }

            // %llx
            if (*format == L'l' && *(format + 1) == L'l' && *(format + 2) == L'x') {
                format += 3;

                unsigned long long val = va_arg(args, unsigned long long);

                CHAR16 tmp[17];
                tmp[16] = L'\0';

                static const CHAR16 hexchars[] = L"0123456789abcdef";

                for (int i = 15; i >= 0; i--) {
                    tmp[i] = hexchars[val & 0xF];
                    val >>= 4;
                }

                int start = 0;
                while (start < 15 && tmp[start] == L'0') start++;

                for (int i = start; i < 16 && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1); i++) {
                    buffer[buf_index++] = tmp[i];
                }

                continue;
            }

            // %lx (FIXED EFI_STATUS SUPPORT)
            if (*format == L'l' && *(format + 1) == L'x') {
                format += 2;

                unsigned long val = va_arg(args, unsigned long);

                CHAR16 tmp[17];
                tmp[16] = L'\0';

                static const CHAR16 hexchars[] = L"0123456789abcdef";

                for (int i = 15; i >= 0; i--) {
                    tmp[i] = hexchars[val & 0xF];
                    val >>= 4;
                }

                int start = 0;
                while (start < 15 && tmp[start] == L'0') start++;

                if (start == 16) start = 15; // ensure at least 0

                for (int i = start; i < 16 && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1); i++) {
                    buffer[buf_index++] = tmp[i];
                }

                continue;
            }

            if (*format == L'0' && *(format + 1) == L'2' && *(format + 2) == L'x') {
                format += 3;

                unsigned int val = va_arg(args, unsigned int);

                CHAR16 tmp[3];
                tmp[2] = L'\0';

                static const CHAR16 hex[] = L"0123456789abcdef";

                tmp[0] = hex[(val >> 4) & 0xF];
                tmp[1] = hex[val & 0xF];

                buffer[buf_index++] = tmp[0];
                buffer[buf_index++] = tmp[1];

                continue;
            }

            switch (*format) {

                case L's': {
                    CHAR16 *str = va_arg(args, CHAR16 *);
                    while (*str && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1))
                        buffer[buf_index++] = *str++;
                    break;
                }

                case L'd': {
                    int val = va_arg(args, int);
                    CHAR16 tmp[32];
                    itow(val, tmp);

                    for (CHAR16 *p = tmp; *p && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1); p++)
                        buffer[buf_index++] = *p;
                    break;
                }

                case L'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    CHAR16 tmp[32];
                    utow(val, tmp);

                    for (CHAR16 *p = tmp; *p && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1); p++)
                        buffer[buf_index++] = *p;
                    break;
                }

                case L'n': {
                    UINTN val = va_arg(args, UINTN);
                    CHAR16 tmp[32];

#if defined(__x86_64__) || defined(_M_X64)
                    ultow((unsigned long long)val, tmp);
#else
                    utow((unsigned int)val, tmp);
#endif

                    for (CHAR16 *p = tmp; *p && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1); p++)
                        buffer[buf_index++] = *p;
                    break;
                }

                case L'c': {
                    buffer[buf_index++] = (CHAR16)va_arg(args, int);
                    break;
                }


                case L'x': {
                    unsigned int val = va_arg(args, unsigned int);

                    CHAR16 tmp[9];
                    tmp[8] = L'\0';

                    static const CHAR16 hexchars[] = L"0123456789abcdef";

                    for (int i = 7; i >= 0; i--) {
                        tmp[i] = hexchars[val & 0xF];
                        val >>= 4;
                    }

                    for (int i = 0; i < 8 && buf_index < (sizeof(buffer) / sizeof(CHAR16) - 1); i++) {
                        buffer[buf_index++] = tmp[i];
                    }

                    break;
                }

                default:
                    buffer[buf_index++] = L'%';
                    buffer[buf_index++] = *format;
                    break;
            }

        } else {
            buffer[buf_index++] = *format;
        }

        format++;
    }

    buffer[buf_index] = L'\0';

    va_end(args);

    gST->ConOut->OutputString(gST->ConOut, buffer);

    return true;
}
