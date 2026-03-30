#pragma once

#include <stdint.h>

// Converts signed int to hex string
char* itoa16(int64_t val, char* buf) {
    char* ptr = buf + 16;  // assume 16-byte buffer for 64-bit hex
    *ptr = '\0';
    uint64_t uval = (uint64_t)val;
    do {
        uint8_t digit = uval & 0xF;
        *--ptr = (digit < 10 ? '0' : 'A' - 10) + digit;
        uval >>= 4;
    } while (uval);
    return ptr;
}

// Converts unsigned int to hex string
char* utoa16(uint64_t val, char* buf) {
    char* ptr = buf + 16;
    *ptr = '\0';
    do {
        uint8_t digit = val & 0xF;
        *--ptr = (digit < 10 ? '0' : 'A' - 10) + digit;
        val >>= 4;
    } while (val);
    return ptr;
}

// Converts unsigned long long to hex string
char* ultoa16(unsigned long long val, char* buf) {
    return utoa16(val, buf);
}
