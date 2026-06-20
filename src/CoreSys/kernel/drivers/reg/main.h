#pragma once

// Converts a CPU register (RAX/RBX/etc. via inline asm selector) into an int output
// NOTE: x86_64 only, GCC/Clang

#include <stdint.h>

typedef enum {
    REG_RAX,
    REG_RBX,
    REG_RCX,
    REG_RDX
} cpu_reg_t;

static inline void reg_to_int(cpu_reg_t reg, int *out)
{
    uint64_t value = 0;

    switch (reg)
    {
        case REG_RAX:
            __asm__ volatile ("mov %%rax, %0" : "=r"(value));
            break;

        case REG_RBX:
            __asm__ volatile ("mov %%rbx, %0" : "=r"(value));
            break;

        case REG_RCX:
            __asm__ volatile ("mov %%rcx, %0" : "=r"(value));
            break;

        case REG_RDX:
            __asm__ volatile ("mov %%rdx, %0" : "=r"(value));
            break;
    }

    *out = (int)value;
}

void print_regs() {
    int rax;
    reg_to_int(REG_RAX, &rax);

    int rbx;
    reg_to_int(REG_RBX, &rbx);

    int rcx;
    reg_to_int(REG_RCX, &rcx);

    int rdx;
    reg_to_int(REG_RDX, &rdx);

    k_trace_n2("RAX: ");
    kprint_int(rax);
    kprint("\n");

    k_trace_n2("RBX: ");
    kprint_int(rbx);
    kprint("\n");

    k_trace_n2("RCX: ");
    kprint_int(rcx);
    kprint("\n");

    k_trace_n2("RDX: ");
    kprint_int(rdx);
    kprint("\n");
}