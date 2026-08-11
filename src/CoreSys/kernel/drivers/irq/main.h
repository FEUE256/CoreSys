#pragma once

#include <drivers/serial/main.h>
#include <drivers/sf/main.h>
#include <drivers/ret/main.h>

/*
 * x86 Exceptions
 */

#define CS_IRQ_DE   0   // #DE Divide Error
#define CS_IRQ_DB   1   // #DB Debug Exception
#define CS_IRQ_NMI  2   // NMI Interrupt
#define CS_IRQ_BP   3   // #BP Breakpoint
#define CS_IRQ_OF   4   // #OF Overflow
#define CS_IRQ_BR   5   // #BR Bound Range Exceeded
#define CS_IRQ_UD   6   // #UD Invalid Opcode
#define CS_IRQ_NM   7   // #NM Device Not Available

#define CS_IRQ_DF   8   // #DF Double Fault
#define CS_IRQ_CS   9   // #CS Coprocessor Segment Overrun (legacy)

#define CS_IRQ_TS   10  // #TS Invalid TSS
#define CS_IRQ_NP   11  // #NP Segment Not Present
#define CS_IRQ_SS   12  // #SS Stack-Segment Fault
#define CS_IRQ_GP   13  // #GP General Protection Fault
#define CS_IRQ_PF   14  // #PF Page Fault

#define CS_IRQ_RS   15  // #RS Reserved

#define CS_IRQ_MF   16  // #MF x87 Floating-Point Error
#define CS_IRQ_AC   17  // #AC Alignment Check
#define CS_IRQ_MC   18  // #MC Machine Check
#define CS_IRQ_XM   19  // #XM SIMD Floating-Point Exception
#define CS_IRQ_VE   20  // #VE Virtualization Exception
#define CS_IRQ_CP   21  // #CP Control Protection Exception

#define CS_IRQ_16   22  // Reserved
#define CS_IRQ_17   23  // Reserved
#define CS_IRQ_18   24  // Reserved
#define CS_IRQ_19   25  // Reserved
#define CS_IRQ_1A   26  // Reserved
#define CS_IRQ_1B   27  // Reserved

#define CS_IRQ_HV   28  // #HV Hypervisor Injection Exception
#define CS_IRQ_VC   29  // #VC VMM Communication Exception
#define CS_IRQ_SX   30  // #SX Security Exception

#define CS_IRQ_1F   31  // Reserved

/*
    If you get qemu defualt = one error (DF for 8)
    If you get IRQ: xxx = duble error (TF for 8)
    If you get IRQ: xxx (#TF) = triple error (Four errors (FF) for 8)
*/

const char* cs_irq_name(int irq)
{
    switch (irq)
    {
        case CS_IRQ_DE:  return "Divide Error (#DE)";
        case CS_IRQ_DB:  return "Debug Exception (#DB)";
        case CS_IRQ_NMI: return "NMI Interrupt";
        case CS_IRQ_BP:  return "Breakpoint (#BP)";
        case CS_IRQ_OF:  return "Overflow (#OF)";
        case CS_IRQ_BR:  return "Bound Range Exceeded (#BR)";
        case CS_IRQ_UD:  return "Invalid Opcode (#UD)";
        case CS_IRQ_NM:  return "Device Not Available (#NM)";
        case CS_IRQ_DF:  return "Double Fault (#DF)";
        case CS_IRQ_CS:  return "Coprocessor Segment Overrun (#CS)";
        case CS_IRQ_TS:  return "Invalid TSS (#TS)";
        case CS_IRQ_NP:  return "Segment Not Present (#NP)";
        case CS_IRQ_SS:  return "Stack-Segment Fault (#SS)";
        case CS_IRQ_GP:  return "General Protection Fault (#GP)";
        case CS_IRQ_PF:  return "Page Fault (#PF)";
        case CS_IRQ_RS:  return "Reserved (#RS)";

        case CS_IRQ_MF:  return "x87 Floating-Point Error (#MF)";
        case CS_IRQ_AC:  return "Alignment Check (#AC)";
        case CS_IRQ_MC:  return "Machine Check (#MC)";
        case CS_IRQ_XM:  return "SIMD Floating-Point Exception (#XM)";
        case CS_IRQ_VE:  return "Virtualization Exception (#VE)";
        case CS_IRQ_CP:  return "Control Protection Exception (#CP)";

        case CS_IRQ_16:  return "Reserved (0x16)";
        case CS_IRQ_17:  return "Reserved (0x17)";
        case CS_IRQ_18:  return "Reserved (0x18)";
        case CS_IRQ_19:  return "Reserved (0x19)";
        case CS_IRQ_1A:  return "Reserved (0x1A)";
        case CS_IRQ_1B:  return "Reserved (0x1B)";

        case CS_IRQ_HV:  return "Hypervisor Injection Exception (#HV)";
        case CS_IRQ_VC:  return "VMM Communication Exception (#VC)";
        case CS_IRQ_SX:  return "Security Exception (#SX)";
        case CS_IRQ_1F:  return "Reserved (0x1F)";

        default:
            return "Unknown";
    }
}

void irq(int irq) {
    const char *string = cs_irq_name(irq);

   if (strcmp((char*)string, "Divide Error (#DE)") == 0) {
        _cs_asm_irq_de();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Debug Exception (#DB)") == 0) {
        _cs_asm_irq_db();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "NMI Interrupt") == 0) {
        _cs_asm_irq_nmi();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Breakpoint (#BP)") == 0) {
        _cs_asm_irq_bp();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Overflow (#OF)") == 0) {
        _cs_asm_irq_of();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Bound Range Exceeded (#BR)") == 0) {
        _cs_asm_irq_br();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Invalid Opcode (#UD)") == 0) {
        _cs_asm_irq_ud();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Device Not Available (#NM)") == 0) {
        _cs_asm_irq_nm();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Double Fault (#DF)") == 0) {
        _cs_asm_irq_df();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Coprocessor Segment Overrun (#CS)") == 0) {
        _cs_asm_irq_cs();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Invalid TSS (#TS)") == 0) {
        _cs_asm_irq_ts();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Segment Not Present (#NP)") == 0) {
        _cs_asm_irq_np();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Stack-Segment Fault (#SS)") == 0) {
        _cs_asm_irq_ss();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "General Protection Fault (#GP)") == 0) {
        _cs_asm_irq_gp();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Page Fault (#PF)") == 0) {
        _cs_asm_irq_pf();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved (#RS)") == 0) {
        _cs_asm_irq_rs();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "x87 Floating-Point Error (#MF)") == 0) {
        _cs_asm_irq_mf();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Alignment Check (#AC)") == 0) {
        _cs_asm_irq_ac();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Machine Check (#MC)") == 0) {
        _cs_asm_irq_mc();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "SIMD Floating-Point Exception (#XM)") == 0) {
        _cs_asm_irq_xm();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Virtualization Exception (#VE)") == 0) {
        _cs_asm_irq_ve();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Control Protection Exception (#CP)") == 0) {
        _cs_asm_irq_cp();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved 0x16") == 0) {
        _cs_asm_irq_16();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved 0x17") == 0) {
        _cs_asm_irq_17();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved 0x18") == 0) {
        _cs_asm_irq_18();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved 0x19") == 0) {
        _cs_asm_irq_19();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved 0x1A") == 0) {
        _cs_asm_irq_1A();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved 0x1B") == 0) {
        _cs_asm_irq_1B();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Hypervisor Injection Exception (#HV)") == 0) {
        _cs_asm_irq_hv();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "VMM Communication Exception (#VC)") == 0) {
        _cs_asm_irq_vc();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Security Exception (#SX)") == 0) {
        _cs_asm_irq_sx();
        k_sff("IRQ: %s :(", string);
    }

    if (strcmp((char*)string, "Reserved 0x1F") == 0) {
        _cs_asm_irq_1F();
        k_sff("IRQ: %s :(", string);
    }

    k_sff("IRQ: %s (#TF)", string);
}
