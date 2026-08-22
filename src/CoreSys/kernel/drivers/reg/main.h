#pragma once

// Converts a CPU register (RAX/RBX/etc. via inline asm selector) into an int output
// NOTE: x86_64 only, GCC/Clang

#include <stdint.h>
#include <asm/global.h>
#include <misc/avx.h>

extern void _cs_asm_get_regs(regs_t *regs);

_Static_assert(offsetof(regs_t, rip) == 0x80, "RIP wrong");
_Static_assert(offsetof(regs_t, rflags) == 0x88, "RFLAGS wrong");

static void print_u128(const char *name, uint128_t v)
{
    kprintf("%s: ", name);
    kprint_u64(v.high);
    kprintf(" ");
    kprint_u64(v.low);
    kprintf("\n");
}

#define PRINT_U64(name, value)       \
    do {                              \
        kprintf(name ": ");           \
        kprint_u64((value));          \
        kprintf("\n");                \
    } while (0)

void print_regs(void)
{
    if (!ksf)
        enable_sse_s();

    if (!ksf)
        sse_init();

    _cs_asm_get_regs(&regs);

    uint64_t *stack = (uint64_t *)regs.rsp;

    PRINT_U64("STACK[0]", stack[0]);
    PRINT_U64("STACK[1]", stack[1]);
    PRINT_U64("STACK[2]", stack[2]);
    PRINT_U64("RAX", regs.rax);
    PRINT_U64("RBX", regs.rbx);
    PRINT_U64("RCX", regs.rcx);
    PRINT_U64("RDX", regs.rdx);
    PRINT_U64("RSI", regs.rsi);
    PRINT_U64("RDI", regs.rdi);
    PRINT_U64("RBP", regs.rbp);
    PRINT_U64("RSP", regs.rsp);
    PRINT_U64("R8",  regs.r8);
    PRINT_U64("R9",  regs.r9);
    PRINT_U64("R10", regs.r10);
    PRINT_U64("R11", regs.r11);
    PRINT_U64("R12", regs.r12);
    PRINT_U64("R13", regs.r13);
    PRINT_U64("R14", regs.r14);
    PRINT_U64("R15", regs.r15);
    PRINT_U64("RIP", regs.rip);
    PRINT_U64("RFLAGS", regs.rflags);
    PRINT_U64("CS", regs.cs);
    PRINT_U64("DS", regs.ds);
    PRINT_U64("ES", regs.es);
    PRINT_U64("SS", regs.ss);
    PRINT_U64("FS", regs.fs);
    PRINT_U64("GS", regs.gs);
    PRINT_U64("CR0", regs.cr0);
    PRINT_U64("CR2", regs.cr2);
    PRINT_U64("CR3", regs.cr3);
    PRINT_U64("CR4", regs.cr4);
    PRINT_U64("CR8", regs.cr8);
    PRINT_U64("DR0", regs.dr0);
    PRINT_U64("DR1", regs.dr1);
    PRINT_U64("DR2", regs.dr2);
    PRINT_U64("DR3", regs.dr3);
    PRINT_U64("DR6", regs.dr6);
    PRINT_U64("DR7", regs.dr7);
    PRINT_U64("TSS_RSP0", regs.tss_rsp0);
    PRINT_U64("TSS_RSP1", regs.tss_rsp1);
    PRINT_U64("TSS_RSP2", regs.tss_rsp2);
    PRINT_U64("TSS_RSP0", regs.tss_rsp0);
    PRINT_U64("TSS_RSP1", regs.tss_rsp1);
    PRINT_U64("TSS_RSP2", regs.tss_rsp2);
    PRINT_U64("TSS_IST1", regs.tss_ist1);
    PRINT_U64("TSS_IST2", regs.tss_ist2);
    PRINT_U64("TSS_IST3", regs.tss_ist3);
    PRINT_U64("TSS_IST4", regs.tss_ist4);
    PRINT_U64("TSS_IST5", regs.tss_ist5);
    PRINT_U64("TSS_IST6", regs.tss_ist6);
    PRINT_U64("TSS_IST7", regs.tss_ist7);
    PRINT_U64("FPU_CONTROL", regs.fpu_control_word);
    PRINT_U64("FPU_STATUS", regs.fpu_status_word);
    PRINT_U64("FPU_TAG", regs.fpu_tag_word);
    PRINT_U64("FPU_OPCODE", regs.fpu_opcode);
    PRINT_U64("FPU_IP", regs.fpu_ip);
    PRINT_U64("FPU_DP", regs.fpu_dp);
    PRINT_U64("FPU_CS", regs.fpu_cs);
    PRINT_U64("FPU_DS", regs.fpu_ds);
    PRINT_U64("MM0", regs.mmx0);
    PRINT_U64("MM1", regs.mmx1);
    PRINT_U64("MM2", regs.mmx2);
    PRINT_U64("MM3", regs.mmx3);
    PRINT_U64("MM4", regs.mmx4);
    PRINT_U64("MM5", regs.mmx5);
    PRINT_U64("MM6", regs.mmx6);
    PRINT_U64("MM7", regs.mmx7);
    PRINT_U64("MXCSR", regs.mxcsr);
    PRINT_U64("MXCSR_MASK", regs.mxcsr_mask);
    print_u128("XMM0",  regs.xmm0);
    print_u128("XMM1",  regs.xmm1);
    print_u128("XMM2",  regs.xmm2);
    print_u128("XMM3",  regs.xmm3);
    print_u128("XMM4",  regs.xmm4);
    print_u128("XMM5",  regs.xmm5);
    print_u128("XMM6",  regs.xmm6);
    print_u128("XMM7",  regs.xmm7);
    print_u128("XMM8",  regs.xmm8);
    print_u128("XMM9",  regs.xmm9);
    print_u128("XMM10", regs.xmm10);
    print_u128("XMM11", regs.xmm11);
    print_u128("XMM12", regs.xmm12);
    print_u128("XMM13", regs.xmm13);
    print_u128("XMM14", regs.xmm14);
    print_u128("XMM15", regs.xmm15);
    PRINT_U64("IA32_EFER", regs.msr_efer);
    PRINT_U64("IA32_STAR", regs.msr_star);
    PRINT_U64("IA32_LSTAR", regs.msr_lstar);
    PRINT_U64("IA32_CSTAR", regs.msr_cstar);
    PRINT_U64("IA32_SFMASK", regs.msr_sfmask);
    PRINT_U64("IA32_KERNEL_GS_BASE", regs.msr_kernel_gs_base);
    PRINT_U64("IA32_APIC_BASE", regs.msr_apic_base);
    PRINT_U64("TLB_BASE", regs.tlb_base);
    PRINT_U64("TLB_ENTRIES", regs.tlb_entries);
}
