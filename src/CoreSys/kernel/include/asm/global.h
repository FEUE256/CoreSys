#pragma once

#define CS_X86_64 

typedef struct regs_t regs_t;

extern void _cs_asm_init_main(void);
extern void _cs_asm_get_regs(regs_t *regs);

// ASM INT 

extern void _cs_asm_irq_de(void);    // 00 (0)  #DE Divide Error
extern void _cs_asm_irq_db(void);    // 01 (1)  #DB Debug Exception
extern void _cs_asm_irq_nmi(void);   // 02 (2)  NMI Interrupt
extern void _cs_asm_irq_bp(void);    // 03 (3)  #BP Breakpoint
extern void _cs_asm_irq_of(void);    // 04 (4)  #OF Overflow
extern void _cs_asm_irq_br(void);    // 05 (5)  #BR Bound Range Exceeded
extern void _cs_asm_irq_ud(void);    // 06 (6)  #UD Invalid Opcode
extern void _cs_asm_irq_nm(void);    // 07 (7)  #NM Device Not Available
extern void _cs_asm_irq_df(void);    // 08 (8)  #DF Double Fault
extern void _cs_asm_irq_cs(void);    // 09 (9)  Coprocessor Segment Overrun (#CS in CoreSyS)
extern void _cs_asm_irq_ts(void);    // 0A (10) #TS Invalid TSS
extern void _cs_asm_irq_np(void);    // 0B (11) #NP Segment Not Present
extern void _cs_asm_irq_ss(void);    // 0C (12) #SS Stack-Segment Fault
extern void _cs_asm_irq_gp(void);    // 0D (13) #GP General Protection Fault
extern void _cs_asm_irq_pf(void);    // 0E (14) #PF Page Fault
extern void _cs_asm_irq_rs(void);    // 0F (15) #RS Reserved
extern void _cs_asm_irq_mf(void);    // 10 (16) #MF x87 Floating-Point Error
extern void _cs_asm_irq_ac(void);    // 11 (17) #AC Alignment Check
extern void _cs_asm_irq_mc(void);    // 12 (18) #MC Machine Check
extern void _cs_asm_irq_xm(void);    // 13 (19) #XM SIMD Floating-Point Exception
extern void _cs_asm_irq_ve(void);    // 14 (20) #VE Virtualization Exception
extern void _cs_asm_irq_cp(void);    // 15 (21) #CP Control Protection Exception
extern void _cs_asm_irq_16(void);    // 16 (22) Reserved
extern void _cs_asm_irq_17(void);    // 17 (23) Reserved
extern void _cs_asm_irq_18(void);    // 18 (24) Reserved
extern void _cs_asm_irq_19(void);    // 19 (25) Reserved
extern void _cs_asm_irq_1A(void);    // 1A (26) Reserved
extern void _cs_asm_irq_1B(void);    // 1B (27) Reserved
extern void _cs_asm_irq_hv(void);    // 1C (28) #HV Hypervisor Injection Exception
extern void _cs_asm_irq_vc(void);    // 1D (29) #VC VMM Communication Exception
extern void _cs_asm_irq_sx(void);    // 1E (30) #SX Security Exception
extern void _cs_asm_irq_1F(void);    // 1F (31) Reserved
