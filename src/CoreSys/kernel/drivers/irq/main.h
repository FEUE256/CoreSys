#pragma once

#include <drivers/serial/main.h>
#include <drivers/sf/main.h>

#pragma once

/*
 * x86 Exceptions
 */

#define CS_IRQ_DE   0   // Divide Error
#define CS_IRQ_DB   1   // Debug
#define CS_IRQ_NMI  2   // Non-Maskable Interrupt
#define CS_IRQ_BP   3   // Breakpoint
#define CS_IRQ_OF   4   // Overflow
#define CS_IRQ_BR   5   // BOUND Range Exceeded
#define CS_IRQ_UD   6   // Invalid Opcode
#define CS_IRQ_NM   7   // Device Not Available

#define CS_IRQ_DF   8   // Double Fault

#define CS_IRQ_CO   9   // Coprocessor Segment Overrun (legacy)

#define CS_IRQ_TS   10  // Invalid TSS
#define CS_IRQ_NP   11  // Segment Not Present
#define CS_IRQ_SS   12  // Stack Segment Fault
#define CS_IRQ_GP   13  // General Protection
#define CS_IRQ_PF   14  // Page Fault

#define CS_IRQ_RS   15  // Reserved

#define CS_IRQ_MF   16  // x87 Floating Point
#define CS_IRQ_AC   17  // Alignment Check
#define CS_IRQ_MC   18  // Machine Check
#define CS_IRQ_XM   19  // SIMD Floating Point
#define CS_IRQ_VE   20  // Virtualization
#define CS_IRQ_CP   21  // Control Protection

#define CS_IRQ_HV   28  // Hypervisor Injection
#define CS_IRQ_VC   29  // VMM Communication
#define CS_IRQ_SX   30  // Security Exception

const char* cs_irq_name(int irq)
{
    switch (irq)
    {
        case CS_IRQ_DE:  return "Divide Error (#DE)";
        case CS_IRQ_DB:  return "Debug (#DB)";
        case CS_IRQ_NMI: return "Non-Maskable Interrupt (NMI)";
        case CS_IRQ_BP:  return "Breakpoint (#BP)";
        case CS_IRQ_OF:  return "Overflow (#OF)";
        case CS_IRQ_BR:  return "BOUND Range Exceeded (#BR)";
        case CS_IRQ_UD:  return "Invalid Opcode (#UD)";
        case CS_IRQ_NM:  return "Device Not Available (#NM)";
        case CS_IRQ_DF:  return "Double Fault (#DF)";
        case CS_IRQ_TS:  return "Invalid TSS (#TS)";
        case CS_IRQ_NP:  return "Segment Not Present (#NP)";
        case CS_IRQ_SS:  return "Stack Segment Fault (#SS)";
        case CS_IRQ_GP:  return "General Protection Fault (#GP)";
        case CS_IRQ_PF:  return "Page Fault (#PF)";
        case CS_IRQ_MF:  return "x87 Floating-Point Exception (#MF)";
        case CS_IRQ_AC:  return "Alignment Check (#AC)";
        case CS_IRQ_MC:  return "Machine Check (#MC)";
        case CS_IRQ_XM:  return "SIMD Floating-Point Exception (#XM)";
        case CS_IRQ_VE:  return "Virtualization Exception (#VE)";
        case CS_IRQ_CP:  return "Control Protection Exception (#CP)";
        case CS_IRQ_HV:  return "Hypervisor Injection Exception (#HV)";
        case CS_IRQ_VC:  return "VMM Communication Exception (#VC)";
        case CS_IRQ_SX:  return "Security Exception (#SX)";

        default:
            return "Reserved/Unknown";
    }
}

void irq(int irq) {
    k_sff("IRQ: %s", cs_irq_name(irq));
}
