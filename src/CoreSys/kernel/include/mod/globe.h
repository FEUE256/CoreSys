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
    // General Purpose Registers
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;

    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    // Instruction state
    uint64_t rip;
    uint64_t rflags;

    // Segment registers
    uint64_t cs;
    uint64_t ds;
    uint64_t es;
    uint64_t ss;
    uint64_t fs;
    uint64_t gs;

    // Control registers
    uint64_t cr0;
    uint64_t cr2;   // Page fault address
    uint64_t cr3;   // Page table base
    uint64_t cr4;
    uint64_t cr8;

    // Debug registers
    uint64_t dr0;
    uint64_t dr1;
    uint64_t dr2;
    uint64_t dr3;
    uint64_t dr6;
    uint64_t dr7;

    // XMM registers
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

} regs_t;

// TODO: FPU/MMX/x87 state MXCSR YMM and ZMM opmask0-7 MSR TLBuint64_t tss_rsp0;
// uint64_t tss_rsp1;
// uint64_t tss_rsp2;

// uint64_t ist1;
// uint64_t ist2;
// uint64_t ist3;
// uint64_t ist4;
// uint64_t ist5;
// uint64_t ist6;
// uint64_t ist7;

// TODO: Software irt 
// 0x80  System call
// 0x81  Kernel debug
// 0x82  Kernel panic
// 0x83  Scheduler request
// 0x84  Yield CPU
// 0x85  Process exit
// 0x86  Spawn process
// 0x87  IPC message
// 0x88  Memory statistics
// 0x89  File system service for exaple

regs_t regs;

bool ksf = 0;

bool cop_g_inited  = false;
bool cop_g_mounted = false;
