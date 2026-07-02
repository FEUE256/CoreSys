#pragma once 

/* =========================
   SAFE GCC / MSVC COMPAT x86 CORE
   ========================= */

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

/* =========================
       SYSTEM HEADERS
   ========================= */

#include <mod/globe.h>
#include <mod/types.h>
#include <mod/assert.h>
#include <mod/status.h>

#include <asm/global.h>

#include <drivers/vga/main.h>

#include <drivers/ret/main.h>
#include <drivers/status/main.h>
#include <drivers/pci/main.h>
#include <drivers/ahci/main.h>
#include <drivers/irq/main.h>
#include <drivers/ata/main.h>

#include <drivers/ACPI/main.h>
#include <drivers/led/main.h>
#include <drivers/serial/main.h>
#include <drivers/time/main.h>

#include <drivers/tty/main.h>
#include <drivers/tty/cmd.h>

#include <drivers/halcalls/main.h>
#include <drivers/hal/main.h>

#include <drivers/init/main.h>
#include <drivers/task/main.h>
#include <drivers/halt/main.h>
#include <drivers/sf/main.h>

#include <drivers/syscalls/main.h>
#include <drivers/sys/main.h>

#include <drivers/cfs/main.h>
#include <drivers/log/main.h>
#include <drivers/page/main.h>
#include <drivers/reg/main.h>

#include <drivers/nvme/main.h>

#include <kernel/mem.h>
