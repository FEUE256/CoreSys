#pragma once

#include <drivers/serial/main.h>
#include <drivers/sf/main.h>

#include <stdint.h>

void k_sff(const char *fmt, ...);

int rdrand64(uint64_t *out)
{
    (void)out; // Gives error else but it is still used in inline asm
    unsigned char ok;

    __asm__ volatile(
        "rdrand %0\n"
        "setc %1"
        : "=r"(*out), "=qm"(ok)
    );

    return ok;
}

uint64_t k_trnd()
{
    uint64_t value;

    int status = rdrand64(&value); 

    while (status == 0)
    {
        k_sff("[KERNEL SERVICE] ERROR: INTEL ASM INSTRUTION 'RDRAND' FAILED WITH ERROR CODE: %s (UNDEFINED INSTRUTION VALUE: %x)", status, (uint32_t)value); // Kernel Faliure
    }
    
    return value;
}