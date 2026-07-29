#pragma once

#include <stdint.h>

#include <drivers/irq/main.h>

// irq numbers (CoreSys ABI)
enum irqcalls {
    /* Software IRQ services */
    IRQ_DEV_NULL = 0x000,

    /* CPU Exceptions 0x100-0x11E */
    IRQ_DE = 0x100,   // #DE Divide Error
    IRQ_DB = 0x101,   // #DB Debug
    IRQ_NMI = 0x102,  // Non-Maskable Interrupt
    IRQ_BP = 0x103,   // #BP Breakpoint
    IRQ_OF = 0x104,   // #OF Overflow
    IRQ_BR = 0x105,   // #BR BOUND Range Exceeded
    IRQ_UD = 0x106,   // #UD Invalid Opcode
    IRQ_NM = 0x107,   // #NM Device Not Available
    IRQ_DF = 0x108,   // #DF Double Fault
    IRQ_CO = 0x109,   // Coprocessor Segment Overrun
    IRQ_TS = 0x10A,   // #TS Invalid TSS
    IRQ_NP = 0x10B,   // #NP Segment Not Present
    IRQ_SS = 0x10C,   // #SS Stack Segment Fault
    IRQ_GP = 0x10D,   // #GP General Protection
    IRQ_PF = 0x10E,   // #PF Page Fault
    IRQ_RS = 0x10F,   // Reserved
    IRQ_MF = 0x110,   // #MF x87 Floating Point
    IRQ_AC = 0x111,   // #AC Alignment Check
    IRQ_MC = 0x112,   // #MC Machine Check
    IRQ_XM = 0x113,   // #XM SIMD Floating Point
    IRQ_VE = 0x114,   // #VE Virtualization
    IRQ_CP = 0x115,   // #CP Control Protection
    IRQ_HV = 0x11C,   // #HV Hypervisor Injection
    IRQ_VC = 0x11D,   // #VC VMM Communication
    IRQ_SX = 0x11E    // #SX Security Exception
};

// irqcalls ABI frame
typedef struct {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
} irqcall_frame_t;

// Driver functions (kernel layer)
void irq(int irq);

// Dispatcher
uint64_t irqcall(irqcall_frame_t *frame)
{
    uint64_t id = frame->rax;

    if (id == IRQ_DEV_NULL) {
        return (uint64_t)irqret((int)frame->rdi);
    }
    else if (id == IRQ_DE) {
        irq(CS_IRQ_DE);
    }
    else if (id == IRQ_DB) {
        irq(CS_IRQ_DB);
    }
    else if (id == IRQ_NMI) {
        irq(CS_IRQ_NMI);
    }
    else if (id == IRQ_BP) {
        irq(CS_IRQ_BP);
    }
    else if (id == IRQ_OF) {
        irq(CS_IRQ_OF);
    }
    else if (id == IRQ_BR) {
        irq(CS_IRQ_BR);
    }
    else if (id == IRQ_UD) {
        irq(CS_IRQ_UD);
    }
    else if (id == IRQ_NM) {
        irq(CS_IRQ_NM);
    }
    else if (id == IRQ_DF) {
        irq(CS_IRQ_DF);
    }
    else if (id == IRQ_CO) {
        irq(CS_IRQ_CO);
    }
    else if (id == IRQ_TS) {
        irq(CS_IRQ_TS);
    }
    else if (id == IRQ_NP) {
        irq(CS_IRQ_NP);
    }
    else if (id == IRQ_SS) {
        irq(CS_IRQ_SS);
    }
    else if (id == IRQ_GP) {
        irq(CS_IRQ_GP);
    }
    else if (id == IRQ_PF) {
        irq(CS_IRQ_PF);
    }
    else if (id == IRQ_RS) {
        irq(CS_IRQ_RS);
    }
    else if (id == IRQ_MF) {
        irq(CS_IRQ_MF);
    }
    else if (id == IRQ_AC) {
        irq(CS_IRQ_AC);
    }
    else if (id == IRQ_MC) {
        irq(CS_IRQ_MC);
    }
    else if (id == IRQ_XM) {
        irq(CS_IRQ_XM);
    }
    else if (id == IRQ_VE) {
        irq(CS_IRQ_VE);
    }
    else if (id == IRQ_CP) {
        irq(CS_IRQ_CP);
    }
    else if (id == IRQ_HV) {
        irq(CS_IRQ_HV);
    }
    else if (id == IRQ_VC) {
        irq(CS_IRQ_VC);
    }
    else if (id == IRQ_SX) {
        irq(CS_IRQ_SX);
    }
    else {
        irq(CS_IRQ_UD); // Invalid IRQ request
    }

    // Dead code (irq -> exception handler -> hlt)

    return (uint64_t)-1;
}

#include <stdint.h>
#include <stddef.h>
#include <drivers/serial/main.h>

// Kernel irqcall entry 
uint64_t irqcall(irqcall_frame_t *frame);

static inline uint64_t do_irqcall(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx) {
    irqcall_frame_t frame;
    frame.rax = rax;
    frame.rdi = rdi;
    frame.rsi = rsi;
    frame.rdx = rdx;
    return irqcall(&frame);
}

uint64_t irq_dev_null(int code)
{
    return do_irqcall(IRQ_DEV_NULL, code, 0, 0);
}

/* CPU Exceptions */

uint64_t irq_de(void)
{
    return do_irqcall(IRQ_DE, 0, 0, 0);
}

uint64_t irq_db(void)
{
    return do_irqcall(IRQ_DB, 0, 0, 0);
}

uint64_t irq_nmi(void)
{
    return do_irqcall(IRQ_NMI, 0, 0, 0);
}

uint64_t irq_bp(void)
{
    return do_irqcall(IRQ_BP, 0, 0, 0);
}

uint64_t irq_of(void)
{
    return do_irqcall(IRQ_OF, 0, 0, 0);
}

uint64_t irq_br(void)
{
    return do_irqcall(IRQ_BR, 0, 0, 0);
}

uint64_t irq_ud(void)
{
    return do_irqcall(IRQ_UD, 0, 0, 0);
}

uint64_t irq_nm(void)
{
    return do_irqcall(IRQ_NM, 0, 0, 0);
}

uint64_t irq_df(void)
{
    return do_irqcall(IRQ_DF, 0, 0, 0);
}

uint64_t irq_co(void)
{
    return do_irqcall(IRQ_CO, 0, 0, 0);
}

uint64_t irq_ts(void)
{
    return do_irqcall(IRQ_TS, 0, 0, 0);
}

uint64_t irq_np(void)
{
    return do_irqcall(IRQ_NP, 0, 0, 0);
}

uint64_t irq_ss(void)
{
    return do_irqcall(IRQ_SS, 0, 0, 0);
}

uint64_t irq_gp(void)
{
    return do_irqcall(IRQ_GP, 0, 0, 0);
}

uint64_t irq_pf(void)
{
    return do_irqcall(IRQ_PF, 0, 0, 0);
}

uint64_t irq_rs(void)
{
    return do_irqcall(IRQ_RS, 0, 0, 0);
}

uint64_t irq_mf(void)
{
    return do_irqcall(IRQ_MF, 0, 0, 0);
}

uint64_t irq_ac(void)
{
    return do_irqcall(IRQ_AC, 0, 0, 0);
}

uint64_t irq_mc(void)
{
    return do_irqcall(IRQ_MC, 0, 0, 0);
}

uint64_t irq_xm(void)
{
    return do_irqcall(IRQ_XM, 0, 0, 0);
}

uint64_t irq_ve(void)
{
    return do_irqcall(IRQ_VE, 0, 0, 0);
}

uint64_t irq_cp(void)
{
    return do_irqcall(IRQ_CP, 0, 0, 0);
}

uint64_t irq_hv(void)
{
    return do_irqcall(IRQ_HV, 0, 0, 0);
}

uint64_t irq_vc(void)
{
    return do_irqcall(IRQ_VC, 0, 0, 0);
}

uint64_t irq_sx(void)
{
    return do_irqcall(IRQ_SX, 0, 0, 0);
}

uint64_t irq_dev_null(int code);
uint64_t irq_de(void);
uint64_t irq_db(void);
uint64_t irq_nmi(void);
uint64_t irq_bp(void);
uint64_t irq_of(void);
uint64_t irq_br(void);
uint64_t irq_ud(void);
uint64_t irq_nm(void);
uint64_t irq_df(void);
uint64_t irq_co(void);
uint64_t irq_ts(void);
uint64_t irq_np(void);
uint64_t irq_ss(void);
uint64_t irq_gp(void);
uint64_t irq_pf(void);
uint64_t irq_rs(void);
uint64_t irq_mf(void);
uint64_t irq_ac(void);
uint64_t irq_mc(void);
uint64_t irq_xm(void);
uint64_t irq_ve(void);
uint64_t irq_cp(void);
uint64_t irq_hv(void);
uint64_t irq_vc(void);
uint64_t irq_sx(void);
