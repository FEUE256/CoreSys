#pragma once

#include <drivers/serial/main.h>
#include <stdint.h>

static inline void cpuid(uint32_t leaf,
                         uint32_t *eax,
                         uint32_t *ebx,
                         uint32_t *ecx,
                         uint32_t *edx)
{
    __asm__ volatile(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

static inline uint64_t xgetbv(uint32_t xcr)
{
    uint32_t eax, edx;

    __asm__ volatile(
        "xgetbv"
        : "=a"(eax), "=d"(edx)
        : "c"(xcr)
    );

    return ((uint64_t)edx << 32) | eax;
}

/*
 * AVX / AVX2 / AVX-512 detection
 *
 * Return values:
 *
 * 0  = No AVX support
 * 1  = CPU supports AVX, but OSXSAVE is disabled
 * 2  = OSXSAVE enabled, but XMM/YMM state is not enabled
 * 3  = AVX is fully supported
 * 4  = AVX2 is fully supported
 * 5  = AVX-512F is fully supported
 */
int cpu_has_avx(void)
{
    uint32_t eax, ebx, ecx, edx;

    /*
     * Check basic AVX support
     */
    cpuid(1, &eax, &ebx, &ecx, &edx);

    // Return 0: CPU does not support AVX
    if (!(ecx & (1u << 28)))
        return 0;


    /*
     * Check OSXSAVE support
     */
    // Return 1: CPU supports AVX but OS cannot save AVX registers
    if (!(ecx & (1u << 27)))
        return 1;


    /*
     * Check XMM/YMM state enabled in XCR0
     */
    uint64_t xcr0 = xgetbv(0);

    // Return 2: OSXSAVE exists but XMM/YMM registers are disabled
    if ((xcr0 & 0x6) != 0x6)
        return 2;


    /*
     * AVX is usable
     */

    /*
     * Check AVX2
     *
     * CPUID(7,0):EBX bit 5
     */
    cpuid(7, &eax, &ebx, &ecx, &edx);

    // Return 4: AVX2 supported and usable
    if (ebx & (1u << 5))
        return 4;


    /*
     * Check AVX-512 Foundation
     *
     * CPUID(7,0):EBX bit 16
     */
    if (ebx & (1u << 16))
    {
        /*
         * AVX-512 requires:
         * XMM state  -> XCR0 bit 1
         * YMM state  -> XCR0 bit 2
         * Opmask    -> XCR0 bit 5
         * ZMM_Hi256 -> XCR0 bit 6
         * Hi16_ZMM  -> XCR0 bit 7
         */
        if ((xcr0 & 0xE6) == 0xE6)
        {
            // Return 5: AVX-512F fully supported and enabled
            return 5;
        }
    }


    // Return 3: AVX supported and usable, but no AVX2/AVX-512 detected
    return 3;
}

static inline uint64_t read_cr4(void)
{
    uint64_t v;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(v));
    return v;
}

static inline void write_cr4(uint64_t v)
{
    __asm__ volatile ("mov %0, %%cr4" :: "r"(v));
}

void enable_osxsave(void)
{
    uint64_t cr4 = read_cr4();

    // CR4.OSXSAVE = bit 18
    cr4 |= (1ULL << 18);

    write_cr4(cr4);
}

static inline void xsetbv(uint32_t index, uint64_t value)
{
    uint32_t eax = value;
    uint32_t edx = value >> 32;

    __asm__ volatile(
        "xsetbv"
        :
        : "a"(eax), "d"(edx), "c"(index)
        : "memory"
    );
}

void enable_avx(void)
{
    // XCR0:
    // bit 0 = x87
    // bit 1 = SSE
    // bit 2 = AVX
    xsetbv(0, 0x7);
}

void avx_enable(void)
{
    uint64_t before = read_cr4();

    kprintf("CR4 before: ");
    kprint_u64(before);
    kprintf("\n");

    enable_osxsave();

    uint64_t after = read_cr4();

    kprintf("CR4 before: ");
    kprint_u64(after);
    kprintf("\n");

    if (after & (1ULL << 18))
        kprintf("OSXSAVE enabled\n");
    else
        kprintf("OSXSAVE FAILED\n");

    {  
        uint64_t before = xgetbv(0);

        kprintf("XCR0 before low: %x\n", (uint32_t)before);

        xsetbv(0, 0x7);

        uint64_t after = xgetbv(0);

        kprintf("XCR0 after low: %x\n", (uint32_t)after);
    }
}

void cpu_avx(void)
{
    int avx = cpu_has_avx();

    if (avx == 0)
    {
        kprintf("AVX: Not supported\n");
    }
    else if (avx == 1)
    {
        kprintf("AVX: CPU supported, but OSXSAVE disabled\n");
    }
    else if (avx == 2)
    {
        kprintf("AVX: OSXSAVE enabled, but XMM/YMM state disabled\n");
    }
    else if (avx == 3)
    {
        kprintf("AVX: Supported and enabled\n");
        kprintf("AVX2: Not detected\n");
        kprintf("AVX-512: Not detected\n");
    }
    else if (avx == 4)
    {
        kprintf("AVX: Supported and enabled\n");
        kprintf("AVX2: Supported and enabled\n");
        kprintf("AVX-512: Not detected\n");
    }
    else if (avx == 5)
    {
        kprintf("AVX: Supported and enabled\n");
        kprintf("AVX2: Supported and enabled\n");
        kprintf("AVX-512F: Supported and enabled\n");
    }
    else
    {
        kprintf("AVX: Unknown detection result (%d)\n", avx);
    }
}

void detect_sse(void)
{
    uint32_t eax, ebx, ecx, edx;

    cpuid(1, &eax, &ebx, &ecx, &edx);

    if (edx & (1 << 25))
        kprintf("SSE: Supported\n");
    else
        kprintf("SSE: Not detected\n");

    if (edx & (1 << 26))
        kprintf("SSE2: Supported\n");
    else
        kprintf("SSE2: Not detected\n");

    if (ecx & (1 << 0))
        kprintf("SSE3: Supported\n");

    if (ecx & (1 << 9))
        kprintf("SSSE3: Supported\n");

    if (ecx & (1 << 19))
        kprintf("SSE4.1: Supported\n");

    if (ecx & (1 << 20))
        kprintf("SSE4.2: Supported\n");
}

static inline void cpuid_s(uint32_t leaf,
                           uint32_t *eax,
                           uint32_t *ebx,
                           uint32_t *ecx,
                           uint32_t *edx)
{
    __asm__ volatile(
        "cpuid"
        : "=a"(*eax),
          "=b"(*ebx),
          "=c"(*ecx),
          "=d"(*edx)
        : "a"(leaf)
    );
}


static inline uint64_t xgetbv_s(uint32_t xcr)
{
    uint32_t eax;
    uint32_t edx;

    __asm__ volatile(
        "xgetbv"
        : "=a"(eax),
          "=d"(edx)
        : "c"(xcr)
    );

    return ((uint64_t)edx << 32) | eax;
}


static inline void xsetbv_s(uint32_t index, uint64_t value)
{
    uint32_t eax = (uint32_t)value;
    uint32_t edx = (uint32_t)(value >> 32);

    __asm__ volatile(
        "xsetbv"
        :
        : "a"(eax),
          "d"(edx),
          "c"(index)
        : "memory"
    );
}


/*
 * AVX detection
 *
 * Return:
 * 0 = No AVX
 * 1 = AVX supported, OSXSAVE disabled
 * 2 = OSXSAVE enabled, XMM/YMM disabled
 * 3 = AVX enabled
 * 4 = AVX2 enabled
 * 5 = AVX-512F enabled
 */
int cpu_has_avx_s(void)
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    cpuid_s(1, &eax, &ebx, &ecx, &edx);

    if (!(ecx & (1u << 28)))
        return 0;

    if (!(ecx & (1u << 27)))
        return 1;


    uint64_t xcr0 = xgetbv_s(0);

    if ((xcr0 & 0x6) != 0x6)
        return 2;


    cpuid_s(7, &eax, &ebx, &ecx, &edx);


    if (ebx & (1u << 5))
        return 4;


    if (ebx & (1u << 16))
    {
        if ((xcr0 & 0xE6) == 0xE6)
            return 5;
    }


    return 3;
}


static inline uint64_t read_cr4_s(void)
{
    uint64_t value;

    __asm__ volatile(
        "mov %%cr4, %0"
        : "=r"(value)
    );

    return value;
}


static inline void write_cr4_s(uint64_t value)
{
    __asm__ volatile(
        "mov %0, %%cr4"
        :
        : "r"(value)
    );
}


void enable_osxsave_s(void)
{
    uint64_t cr4 = read_cr4_s();

    cr4 |= (1ULL << 18);

    write_cr4_s(cr4);
}


void enable_avx_s(void)
{
    /*
     * XCR0:
     * bit 0 = x87
     * bit 1 = SSE
     * bit 2 = AVX
     */
    xsetbv_s(0, 0x7);
}


int avx_enable_s(void)
{
    enable_osxsave_s();

    uint64_t cr4 = read_cr4_s();

    if (!(cr4 & (1ULL << 18)))
        return -1;


    enable_avx_s();


    uint64_t xcr0 = xgetbv_s(0);

    if ((xcr0 & 0x7) != 0x7)
        return -2;


    return 0;
}


uint32_t detect_sse_s(void)
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t flags = 0;


    cpuid_s(1, &eax, &ebx, &ecx, &edx);


    if (edx & (1 << 25))
        flags |= (1 << 0); // SSE


    if (edx & (1 << 26))
        flags |= (1 << 1); // SSE2


    if (ecx & (1 << 0))
        flags |= (1 << 2); // SSE3


    if (ecx & (1 << 9))
        flags |= (1 << 3); // SSSE3


    if (ecx & (1 << 19))
        flags |= (1 << 4); // SSE4.1


    if (ecx & (1 << 20))
        flags |= (1 << 5); // SSE4.2


    return flags;
}

static inline uint64_t read_cr0_s(void)
{
    uint64_t value;

    __asm__ volatile(
        "mov %%cr0, %0"
        : "=r"(value)
    );

    return value;
}


static inline void write_cr0_s(uint64_t value)
{
    __asm__ volatile(
        "mov %0, %%cr0"
        :
        : "r"(value)
    );
}


void enable_sse_s(void)
{
    uint64_t cr0;
    uint64_t cr4;


    /*
     * Enable FPU
     *
     * CR0.EM = 0
     */
    cr0 = read_cr0_s();

    cr0 &= ~(1ULL << 2); // Clear EM

    /*
     * CR0.MP = 1
     */
    cr0 |= (1ULL << 1);

    write_cr0_s(cr0);



    /*
     * Enable SSE state management
     *
     * CR4.OSFXSR = 1
     * CR4.OSXMMEXCPT = 1
     */
    cr4 = read_cr4_s();

    cr4 |= (1ULL << 9);  // OSFXSR
    cr4 |= (1ULL << 10); // OSXMMEXCPT

    write_cr4_s(cr4);
}

void sse_init(void)
{
    uint32_t mxcsr = 0x1F80;

    __asm__ volatile(
        "ldmxcsr %0"
        :
        : "m"(mxcsr)
    );
}
