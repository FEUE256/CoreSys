#pragma once

void print_compiler_info(void)
{
    kprintf("__FILE__ = %s\n", __FILE__);
    kprintf("__LINE__ = %d\n", __LINE__);
    kprintf("__DATE__ = %s\n", __DATE__);
    kprintf("__TIME__ = %s\n", __TIME__);

    #ifdef __STDC__
        kprintf("__STDC__ = %d\n", __STDC__);
    #endif

    #ifdef __STDC_VERSION__
        kprintf("__STDC_VERSION__ = ");
        kprint_u64(__STDC_VERSION__);
        kprintf("\n");
    #endif

    #ifdef __STDC_HOSTED__
        kprintf("__STDC_HOSTED__ = %d\n", __STDC_HOSTED__);
    #endif

    #ifdef __STDC_MB_MIGHT_NEQ_WC__
        kprintf("__STDC_MB_MIGHT_NEQ_WC__ = %d\n", __STDC_MB_MIGHT_NEQ_WC__);
    #endif

    #ifdef __STDC_UTF_16__
        kprintf("__STDC_UTF_16__ = %d\n", __STDC_UTF_16__);
    #endif

    #ifdef __STDC_UTF_32__
        kprintf("__STDC_UTF_32__ = %d\n", __STDC_UTF_32__);
    #endif

    #ifdef __STDC_ANALYZABLE__
        kprintf("__STDC_ANALYZABLE__ = %d\n", __STDC_ANALYZABLE__);
    #endif

    #ifdef __STDC_IEC_559__
        kprintf("__STDC_IEC_559__ = %d\n", __STDC_IEC_559__);
    #endif

    #ifdef __STDC_IEC_559_COMPLEX__
        kprintf("__STDC_IEC_559_COMPLEX__ = %d\n", __STDC_IEC_559_COMPLEX__);
    #endif

    #ifdef __STDC_LIB_EXT1__
        kprintf("__STDC_LIB_EXT1__ = ");
        kprint_u64(__STDC_LIB_EXT1__);
        kprintf("\n");
    #endif

    #ifdef __STDC_NO_ATOMICS__
        kprintf("__STDC_NO_ATOMICS__ = %d\n", __STDC_NO_ATOMICS__);
    #endif

    #ifdef __STDC_NO_COMPLEX__
        kprintf("__STDC_NO_COMPLEX__ = %d\n", __STDC_NO_COMPLEX__);
    #endif

    #ifdef __STDC_NO_THREADS__
        kprintf("__STDC_NO_THREADS__ = %d\n", __STDC_NO_THREADS__);
    #endif

    #ifdef __STDC_NO_VLA__
        kprintf("__STDC_NO_VLA__ = %d\n", __STDC_NO_VLA__);
    #endif

        kprintf("__func__ = %s\n", __func__);


    /* GCC identification */

    #ifdef __GNUC__
        kprintf("__GNUC__ = %d\n", __GNUC__);
    #endif

    #ifdef __GNUC_MINOR__
        kprintf("__GNUC_MINOR__ = %d\n", __GNUC_MINOR__);
    #endif

    #ifdef __GNUC_PATCHLEVEL__
        kprintf("__GNUC_PATCHLEVEL__ = %d\n", __GNUC_PATCHLEVEL__);
    #endif

    #ifdef __VERSION__
        kprintf("__VERSION__ = %s\n", __VERSION__);
    #endif


    /* GCC extensions */

    #ifdef __GNUC_GNU_INLINE__
        kprintf("__GNUC_GNU_INLINE__ = %d\n", __GNUC_GNU_INLINE__);
    #endif

    #ifdef __GNUC_STDC_INLINE__
        kprintf("__GNUC_STDC_INLINE__ = %d\n", __GNUC_STDC_INLINE__);
    #endif

    #ifdef __GNUC_RH__
        kprintf("__GNUC_RH__ = %d\n", __GNUC_RH__);
    #endif


    /* Architecture */

    #ifdef __x86_64__
        kprintf("__x86_64__ = enabled\n");
    #endif

    #ifdef __i386__
        kprintf("__i386__ = enabled\n");
    #endif

    #ifdef __aarch64__
        kprintf("__aarch64__ = enabled\n");
    #endif

    #ifdef __arm__
        kprintf("__arm__ = enabled\n");
    #endif


    /* Operating system */

    #ifdef __linux__
        kprintf("__linux__ = enabled\n");
    #endif

    #ifdef __gnu_linux__
        kprintf("__gnu_linux__ = enabled\n");
    #endif

    #ifdef __unix__
        kprintf("__unix__ = enabled\n");
    #endif


    /* Endianness */

    #ifdef __ORDER_LITTLE_ENDIAN__
        kprintf("__ORDER_LITTLE_ENDIAN__ = %d\n", __ORDER_LITTLE_ENDIAN__);
    #endif

    #ifdef __ORDER_BIG_ENDIAN__
        kprintf("__ORDER_BIG_ENDIAN__ = %d\n", __ORDER_BIG_ENDIAN__);
    #endif

    #ifdef __BYTE_ORDER__
        kprintf("__BYTE_ORDER__ = %d\n", __BYTE_ORDER__);
    #endif


    /* Compiler features */

    #ifdef __OPTIMIZE__
        kprintf("__OPTIMIZE__ = enabled\n");
    #endif

    #ifdef __OPTIMIZE_SIZE__
        kprintf("__OPTIMIZE_SIZE__ = enabled\n");
    #endif

    #ifdef __NO_INLINE__
        kprintf("__NO_INLINE__ = enabled\n");
    #endif

    #ifdef __GNUC_EXECUTION_CHARSET_NAME
        kprintf("__GNUC_EXECUTION_CHARSET_NAME = %s\n",
                __GNUC_EXECUTION_CHARSET_NAME);
    #endif

    #ifdef __GNUC_WIDE_EXECUTION_CHARSET_NAME
        kprintf("__GNUC_WIDE_EXECUTION_CHARSET_NAME = %s\n",
                __GNUC_WIDE_EXECUTION_CHARSET_NAME);
    #endif


    /* Debug */

    #ifdef __DEBUG__
        kprintf("__DEBUG__ = enabled\n");
    #endif

    #ifdef __SANITIZE_ADDRESS__
        kprintf("__SANITIZE_ADDRESS__ = enabled\n");
    #endif

    #ifdef __SANITIZE_THREAD__
        kprintf("__SANITIZE_THREAD__ = enabled\n");
    #endif


    /* C++ compatibility (if compiling C++) */

    #ifdef __cplusplus
        kprintf("__cplusplus = %ld\n", __cplusplus);
    #endif
}

void compile() {
    kprintf("Kernel Compiled at: %s, %s\n", __DATE__, __TIME__);
}
