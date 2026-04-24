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
bool printf(const CHAR16 *format, ...) {
    UINTN row = gST->ConOut->Mode->CursorRow;
    gST->ConOut->SetCursorPosition(gST->ConOut, 0, row);

    va_list args;
    va_start(args, format);

    CHAR16 buffer[1024];
    UINTN buf_index = 0;

    while (*format && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1) {
        if (*format == L'%') {
            format++; // skip '%'

            // Special case: %04x
            if (*format == L'0' && *(format+1) == L'4' && *(format+2) == L'x') {
                format += 3;
                unsigned int val = va_arg(args, unsigned int);
                CHAR16 numbuf[5];
                numbuf[4] = L'\0';
                static const CHAR16 hexchars[] = L"0123456789abcdef";
                for (int i = 3; i >= 0; i--) {
                    numbuf[i] = hexchars[val & 0xF];
                    val >>= 4;
                }
                for (int i = 0; numbuf[i] && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1; i++)
                    buffer[buf_index++] = numbuf[i];
                continue;
            }

            // Special case: %llx
            if (*format == L'l' && *(format+1) == L'l' && *(format+2) == L'x') {
                format += 3;
                unsigned long long val = va_arg(args, unsigned long long);
                CHAR16 numbuf[17];
                numbuf[16] = L'\0';
                static const CHAR16 hexchars[] = L"0123456789abcdef";
                for (int i = 15; i >= 0; i--) {
                    numbuf[i] = hexchars[val & 0xF];
                    val >>= 4;
                }
                int start = 0;
                while (start < 15 && numbuf[start] == L'0') start++;
                for (int i = start; numbuf[i] && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1; i++)
                    buffer[buf_index++] = numbuf[i];
                continue;
            }

            // Standard formats
            switch (*format) {
                case L's': {
                    CHAR16 *str = va_arg(args, CHAR16 *);
                    while (*str && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1)
                        buffer[buf_index++] = *str++;
                    break;
                }
                case L'a': {
                    char *astr = va_arg(args, char *);
                    while (*astr && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1)
                        buffer[buf_index++] = (CHAR16)(*astr++);
                    break;
                }
                case L'd': {
                    int val = va_arg(args, int);
                    CHAR16 numbuf[20];
                    itow(val, numbuf);
                    for (CHAR16 *p = numbuf; *p && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1; p++)
                        buffer[buf_index++] = *p;
                    break;
                }
                case L'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    CHAR16 numbuf[20];
                    utow(val, numbuf);
                    for (CHAR16 *p = numbuf; *p && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1; p++)
                        buffer[buf_index++] = *p;
                    break;
                }
                case L'n': {
                    UINTN val = va_arg(args, UINTN);
                    CHAR16 numbuf[32];
                #if defined(__x86_64__) || defined(_M_X64)
                    ultow((unsigned long long)val, numbuf);
                #else
                    utow((unsigned int)val, numbuf);
                #endif
                    for (CHAR16 *p = numbuf; *p && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1; p++)
                        buffer[buf_index++] = *p;
                    break;
                }
                case L'l': {
                    format++;
                    if (*format == L'u') {
                        unsigned long val = va_arg(args, unsigned long);
                        CHAR16 numbuf[32];
                        ultow(val, numbuf);
                        for (CHAR16 *p = numbuf; *p && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1; p++)
                            buffer[buf_index++] = *p;
                        format++; // skip 'u'
                    } else {
                        buffer[buf_index++] = L'%';
                        buffer[buf_index++] = L'l';
                    }
                    break;
                }
                case L'c': {
                    CHAR16 ch = (CHAR16)va_arg(args, int);
                    buffer[buf_index++] = ch;
                    break;
                }
                case L'%':
                    buffer[buf_index++] = L'%';
                    break;
                case L'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    CHAR16 numbuf[9];
                    numbuf[8] = L'\0';
                    static const CHAR16 hexchars[] = L"0123456789abcdef";
                    for (int i = 7; i >= 0; i--) {
                        numbuf[i] = hexchars[val & 0xF];
                        val >>= 4;
                    }
                    for (int i = 0; numbuf[i] && buf_index < sizeof(buffer)/sizeof(CHAR16) - 1; i++)
                        buffer[buf_index++] = numbuf[i];
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

    cout->OutputString(cout, buffer);
    return true;
}
