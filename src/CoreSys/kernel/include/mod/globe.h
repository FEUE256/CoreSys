#pragma once 

#include <types.h>
#include <status.h>

typedef struct CS_CORE CS_CORE;

// For cfs_node*
#define cfs_MAX_NAME 64
#define cfs_MAX_CHILDREN 16

typedef enum cfs_type { 
    cfs_FILE, 
    cfs_DIR 
} cfs_type;

typedef struct cfs_node {
    let_t name[cfs_MAX_NAME];
    cfs_type type;

    unum64_t size;
    unum64_t* data;

    str_t cfs_node* parent;

    str_t cfs_node* children[cfs_MAX_CHILDREN];
    unum32_t child_count;
} cfs_node;

num_t tsk_ready = 0;

CS_STATUS_T status; // status
CS_SUBSYS_STATUS_T sstatus;

sta_t cfs_node* cfs_root = 0;

sta_t cfs_node* sys_dir = 0;
sta_t cfs_node* kernel_dir = 0;
sta_t cfs_node* system_dir = 0;
sta_t cfs_node* kernel_cfg_file = 0;
sta_t cfs_node* debug_cfg_file = 0;

unum32_t bar2;
unumarr_t fb;
vol_t unum32_t* framebuffer;

#define VGA_GRAPHICS ((volatile uint8_t*)0xA0000) // Mode 13h framebuffer
#define VGA_TEXT     ((volatile uint16_t*)0xB8000) // Text mode memory
#define VGA_MONO     ((volatile uint16_t*)0xB0000) // Monochrome text

num_t rkc_mode = 0;

CS_CORE g_core; // Inited in kmain after RKC segment

typedef struct regs_t {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;

    uint64_t rip;
    uint64_t rflags;

    uint64_t cs, ds, es, ss, fs, gs;

    uint64_t cr0;
    uint64_t cr2;
    uint64_t cr3;
    uint64_t cr4;
    uint64_t cr8;

    uint64_t dr0;
    uint64_t dr1;
    uint64_t dr2;
    uint64_t dr3;
    uint64_t dr6;
    uint64_t dr7;

    uint64_t tss_rsp0;
    uint64_t tss_rsp1;
    uint64_t tss_rsp2;

    uint16_t fpu_control_word;
    uint16_t fpu_status_word;
    uint16_t fpu_tag_word;
    uint16_t fpu_opcode;

    uint64_t fpu_ip;
    uint64_t fpu_dp;

    uint16_t fpu_cs;
    uint16_t fpu_ds;

    uint64_t mmx0;
    uint64_t mmx1;
    uint64_t mmx2;
    uint64_t mmx3;
    uint64_t mmx4;
    uint64_t mmx5;
    uint64_t mmx6;
    uint64_t mmx7;

    uint32_t mxcsr;
    uint32_t mxcsr_mask;

    uint128_t xmm0;
    uint128_t xmm1;
    uint128_t xmm2;
    uint128_t xmm3;
    uint128_t xmm4;
    uint128_t xmm5;
    uint128_t xmm6;
    uint128_t xmm7;
    uint128_t xmm8;
    uint128_t xmm9;
    uint128_t xmm10;
    uint128_t xmm11;
    uint128_t xmm12;
    uint128_t xmm13;
    uint128_t xmm14;
    uint128_t xmm15;

    uint64_t msr_efer;
    uint64_t msr_star;
    uint64_t msr_lstar;
    uint64_t msr_cstar;
    uint64_t msr_sfmask;
    uint64_t msr_kernel_gs_base;
    uint64_t msr_apic_base;

    uint64_t tlb_base;
    uint64_t tlb_entries;

    uint64_t tss_ist1;
    uint64_t tss_ist2;
    uint64_t tss_ist3;
    uint64_t tss_ist4;
    uint64_t tss_ist5;
    uint64_t tss_ist6;
    uint64_t tss_ist7;
} regs_t;

regs_t regs;

bool ksf = 0;

bool cop_g_inited  = false;
bool cop_g_mounted = false;
