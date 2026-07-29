#pragma once

#include <misc/avx.h>

void cpu_id(void)
{
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];

    cpuid(0, &eax, &ebx, &ecx, &edx);

    vendor[0]  = (char)(ebx);
    vendor[1]  = (char)(ebx >> 8);
    vendor[2]  = (char)(ebx >> 16);
    vendor[3]  = (char)(ebx >> 24);

    vendor[4]  = (char)(edx);
    vendor[5]  = (char)(edx >> 8);
    vendor[6]  = (char)(edx >> 16);
    vendor[7]  = (char)(edx >> 24);

    vendor[8]  = (char)(ecx);
    vendor[9]  = (char)(ecx >> 8);
    vendor[10] = (char)(ecx >> 16);
    vendor[11] = (char)(ecx >> 24);

    vendor[12] = '\0';

    kprintf(vendor);
    kprintf("\n");
}
