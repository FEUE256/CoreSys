#pragma once

// NE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdnoreturn.h>

/* x86 baseline intrinsics */
#include <x86intrin.h>
#include <x86gprintrin.h>
#include <immintrin.h>

#include <emmintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <wmmintrin.h>
#include <immintrin.h>

#include <mm_malloc.h>
#include <limits.h>
#include <float.h>
#include <syslimits.h>

/* optional safe CPU features */
#include <cpuid.h>
#include <lzcntintrin.h>
#include <popcntintrin.h>
#include <rdseedintrin.h>
#include <rtmintrin.h>
#include <waitpkgintrin.h>
#include <fxsrintrin.h>

/* debug / toolchain */
#include <gcov.h>
#include <unwind.h>

/* OpenMP (only if you really use it) */
#include <omp.h>
