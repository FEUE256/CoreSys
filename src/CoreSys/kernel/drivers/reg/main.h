#pragma once

// Converts a CPU register (RAX/RBX/etc. via inline asm selector) into an int output
// NOTE: x86_64 only, GCC/Clang

#include <stdint.h>
#include <asm/global.h>
#include <misc/avx.h>

extern void _cs_asm_get_regs(regs_t *regs);

_Static_assert(offsetof(regs_t, rip) == 0x80, "RIP wrong");
_Static_assert(offsetof(regs_t, rflags) == 0x88, "RFLAGS wrong");

_Static_assert(offsetof(regs_t, xmm0) == 0x118, "XMM0 wrong");
_Static_assert(offsetof(regs_t, xmm15) == 0x208, "XMM15 wrong");

void print_regs(void)
{
    if (!ksf) enable_sse_s();  // only safe outside fault path
    if (!ksf) sse_init();

    _cs_asm_get_regs(&regs);

    uint64_t *stack = (uint64_t *)regs.rsp;

    kprintf("STACK[0] = ");
    kprint_u64(stack[0]);

    kprintf("\nSTACK[1] = ");
    kprint_u64(stack[1]);

    kprintf("\nSTACK[2] = ");
    kprint_u64(stack[2]);

    kprintf("\n");

    kprintf("RAX: ");    kprint_u64(regs.rax);    kprintf("\n");
    kprintf("RBX: ");    kprint_u64(regs.rbx);    kprintf("\n");
    kprintf("RCX: ");    kprint_u64(regs.rcx);    kprintf("\n");
    kprintf("RDX: ");    kprint_u64(regs.rdx);    kprintf("\n");

    kprintf("RSI: ");    kprint_u64(regs.rsi);    kprintf("\n");
    kprintf("RDI: ");    kprint_u64(regs.rdi);    kprintf("\n");
    kprintf("RBP: ");    kprint_u64(regs.rbp);    kprintf("\n");
    kprintf("RSP: ");    kprint_u64(regs.rsp);    kprintf("\n");

    kprintf("R8:  ");    kprint_u64(regs.r8);     kprintf("\n");
    kprintf("R9:  ");    kprint_u64(regs.r9);     kprintf("\n");
    kprintf("R10: ");    kprint_u64(regs.r10);    kprintf("\n");
    kprintf("R11: ");    kprint_u64(regs.r11);    kprintf("\n");
    kprintf("R12: ");    kprint_u64(regs.r12);    kprintf("\n");
    kprintf("R13: ");    kprint_u64(regs.r13);    kprintf("\n");
    kprintf("R14: ");    kprint_u64(regs.r14);    kprintf("\n");
    kprintf("R15: ");    kprint_u64(regs.r15);    kprintf("\n");

    kprintf("RIP: ");    kprint_u64(regs.rip);    kprintf("\n");
    kprintf("RFLAGS: "); kprint_u64(regs.rflags); kprintf("\n");

    kprintf("CS: ");     kprint_u64(regs.cs);     kprintf("\n");
    kprintf("DS: ");     kprint_u64(regs.ds);     kprintf("\n");
    kprintf("ES: ");     kprint_u64(regs.es);     kprintf("\n");
    kprintf("SS: ");     kprint_u64(regs.ss);     kprintf("\n");
    kprintf("FS: ");     kprint_u64(regs.fs);     kprintf("\n");
    kprintf("GS: ");     kprint_u64(regs.gs);     kprintf("\n");

    kprintf("CR0: ");    kprint_u64(regs.cr0);    kprintf("\n");
    kprintf("CR2: ");    kprint_u64(regs.cr2);    kprintf("\n");
    kprintf("CR3: ");    kprint_u64(regs.cr3);    kprintf("\n");
    kprintf("CR4: ");    kprint_u64(regs.cr4);    kprintf("\n");
    kprintf("CR8: ");    kprint_u64(regs.cr8);    kprintf("\n");

    kprintf("DR0: ");    kprint_u64(regs.dr0);    kprintf("\n");
    kprintf("DR1: ");    kprint_u64(regs.dr1);    kprintf("\n");
    kprintf("DR2: ");    kprint_u64(regs.dr2);    kprintf("\n");
    kprintf("DR3: ");    kprint_u64(regs.dr3);    kprintf("\n");
    kprintf("DR6: ");    kprint_u64(regs.dr6);    kprintf("\n");
    kprintf("DR7: ");    kprint_u64(regs.dr7);    kprintf("\n");

    kprintf("XMM0:  ");  kprint_u64(regs.xmm0.high);  kprint_u64(regs.xmm0.low);  kprintf("\n");
    kprintf("XMM1:  ");  kprint_u64(regs.xmm1.high);  kprint_u64(regs.xmm1.low);  kprintf("\n");
    kprintf("XMM2:  ");  kprint_u64(regs.xmm2.high);  kprint_u64(regs.xmm2.low);  kprintf("\n");
    kprintf("XMM3:  ");  kprint_u64(regs.xmm3.high);  kprint_u64(regs.xmm3.low);  kprintf("\n");

    kprintf("XMM4:  ");  kprint_u64(regs.xmm4.high);  kprint_u64(regs.xmm4.low);  kprintf("\n");
    kprintf("XMM5:  ");  kprint_u64(regs.xmm5.high);  kprint_u64(regs.xmm5.low);  kprintf("\n");
    kprintf("XMM6:  ");  kprint_u64(regs.xmm6.high);  kprint_u64(regs.xmm6.low);  kprintf("\n");
    kprintf("XMM7:  ");  kprint_u64(regs.xmm7.high);  kprint_u64(regs.xmm7.low);  kprintf("\n");

    kprintf("XMM8:  ");  kprint_u64(regs.xmm8.high);  kprint_u64(regs.xmm8.low);  kprintf("\n");
    kprintf("XMM9:  ");  kprint_u64(regs.xmm9.high);  kprint_u64(regs.xmm9.low);  kprintf("\n");
    kprintf("XMM10: ");  kprint_u64(regs.xmm10.high); kprint_u64(regs.xmm10.low); kprintf("\n");
    kprintf("XMM11: ");  kprint_u64(regs.xmm11.high); kprint_u64(regs.xmm11.low); kprintf("\n");

    kprintf("XMM12: ");  kprint_u64(regs.xmm12.high); kprint_u64(regs.xmm12.low); kprintf("\n");
    kprintf("XMM13: ");  kprint_u64(regs.xmm13.high); kprint_u64(regs.xmm13.low); kprintf("\n");
    kprintf("XMM14: ");  kprint_u64(regs.xmm14.high); kprint_u64(regs.xmm14.low); kprintf("\n");
    kprintf("XMM15: ");  kprint_u64(regs.xmm15.high); kprint_u64(regs.xmm15.low); kprintf("\n");
}
