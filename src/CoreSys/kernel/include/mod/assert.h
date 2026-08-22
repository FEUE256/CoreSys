#pragma once

#include <drivers/sf/main.h>
#include <mod/types.h>
#include <mod/globe.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define assert(expr) do {                     \
    if (!(expr)) {                            \
        k_sf("ASSERT: " #expr                 \
             " at " __FILE__ ":" STR(__LINE__)); \
    }                                         \
} while (0)

#define assert_hard(expr) do { \
    k_sf("ASSERT: " #expr " at " __FILE__ ":" STR(__LINE__)); \
} while (0)

_Static_assert(offsetof(regs_t, rax)             == 0,   "regs_t: rax");
_Static_assert(offsetof(regs_t, rsp)             == 56,  "regs_t: rsp");
_Static_assert(offsetof(regs_t, rip)             == 128, "regs_t: rip");
_Static_assert(offsetof(regs_t, rflags)          == 136, "regs_t: rflags");
_Static_assert(offsetof(regs_t, cr0)             == 192, "regs_t: cr0");
_Static_assert(offsetof(regs_t, dr0)             == 232, "regs_t: dr0");
_Static_assert(offsetof(regs_t, fpu_control_word)== 304, "regs_t: fpu");
_Static_assert(offsetof(regs_t, mxcsr)           == 400, "regs_t: mxcsr");
_Static_assert(offsetof(regs_t, xmm0)            == 408, "regs_t: xmm0");
_Static_assert(offsetof(regs_t, msr_efer)        == 664, "regs_t: msr");
_Static_assert(offsetof(regs_t, tlb_base)        == 720, "regs_t: tlb");
_Static_assert(sizeof(regs_t)                    == 792, "regs_t size mismatch");
