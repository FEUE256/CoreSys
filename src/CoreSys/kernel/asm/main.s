; ============================================================
; CoreSys
; Kernel register state
; x86-64 / NASM
;
; Requirements:
;   CPL0
;   SSE enabled
;   AVX/AVX-512 enabled if collecting YMM/ZMM/opmask state
;
; C ABI:
;   RCX = regs_t *
;
; IMPORTANT:
;   The C structure layout below is the ABI contract.
; ============================================================

extern kaprint

section .bss
    align 16

    global tss_ist1
    tss_ist1: resq 1

    global tss_ist2
    tss_ist2: resq 1

    global tss_ist3
    tss_ist3: resq 1

    global tss_ist4
    tss_ist4: resq 1

    global tss_ist5
    tss_ist5: resq 1

    global tss_ist6
    tss_ist6: resq 1

    global tss_ist7
    tss_ist7: resq 1

stack_bottom:
    resb 0x100000
stack_top:

section .data
    global _cs_asm_init_main
    global _cs_asm_stack_main
    global _cs_asm_get_regs

section .text

; ============================================================
; Initialization print
; ============================================================

init_print:
    mov al, '['
    call kaprint
    mov al, 'L'
    call kaprint
    mov al, 'O'
    call kaprint
    mov al, 'G'
    call kaprint
    mov al, ']'
    call kaprint
    mov al, ' '
    call kaprint

    mov al, 'C'
    call kaprint
    mov al, 'o'
    call kaprint
    mov al, 'r'
    call kaprint
    mov al, 'e'
    call kaprint
    mov al, 'S'
    call kaprint
    mov al, 'y'
    call kaprint
    mov al, 's'
    call kaprint
    mov al, ' '
    call kaprint
    mov al, 'K'
    call kaprint
    mov al, 'e'
    call kaprint
    mov al, 'r'
    call kaprint
    mov al, 'n'
    call kaprint
    mov al, 'e'
    call kaprint
    mov al, 'l'
    call kaprint
    mov al, ' '
    call kaprint
    mov al, 'A'
    call kaprint
    mov al, 'S'
    call kaprint
    mov al, 'M'
    call kaprint
    mov al, ' '
    call kaprint
    mov al, 'i'
    call kaprint
    mov al, 's'
    call kaprint
    mov al, ' '
    call kaprint
    mov al, 'w'
    call kaprint
    mov al, 'o'
    call kaprint
    mov al, 'r'
    call kaprint
    mov al, 'k'
    call kaprint
    mov al, 'i'
    call kaprint
    mov al, 'n'
    call kaprint
    mov al, 'g'
    call kaprint

    mov al, 13
    call kaprint
    mov al, 10
    call kaprint

    ret


_cs_asm_init_main:
    call init_print
    ret
; ============================================================
; _cs_asm_get_regs
;
; RCX = regs_t *
;
; regs_t offsets
; ============================================================

%define R_RAX       0
%define R_RBX       8
%define R_RCX       16
%define R_RDX       24
%define R_RSI       32
%define R_RDI       40
%define R_RBP       48
%define R_RSP       56
%define R_R8        64
%define R_R9        72
%define R_R10       80
%define R_R11       88
%define R_R12       96
%define R_R13       104
%define R_R14       112
%define R_R15       120

%define R_RIP       128
%define R_RFLAGS    136

%define R_CS        144
%define R_DS        152
%define R_ES        160
%define R_SS        168
%define R_FS        176
%define R_GS        184

%define R_CR0       192
%define R_CR2       200
%define R_CR3       208
%define R_CR4       216
%define R_CR8       224

%define R_DR0       232
%define R_DR1       240
%define R_DR2       248
%define R_DR3       256
%define R_DR6       264
%define R_DR7       272

%define R_TSS_RSP0  280
%define R_TSS_RSP1  288
%define R_TSS_RSP2  296

%define R_FPU_CW    304
%define R_FPU_SW    306
%define R_FPU_TW    308
%define R_FPU_OPCODE 310
%define R_FPU_IP    312
%define R_FPU_DP    320
%define R_FPU_CS    328
%define R_FPU_DS    330

%define R_MMX0      336
%define R_MMX1      344
%define R_MMX2      352
%define R_MMX3      360
%define R_MMX4      368
%define R_MMX5      376
%define R_MMX6      384
%define R_MMX7      392

%define R_MXCSR     400
%define R_MXCSR_MASK 404

%define R_XMM0      408
%define R_XMM1      424
%define R_XMM2      440
%define R_XMM3      456
%define R_XMM4      472
%define R_XMM5      488
%define R_XMM6      504
%define R_XMM7      520
%define R_XMM8      536
%define R_XMM9      552
%define R_XMM10     568
%define R_XMM11     584
%define R_XMM12     600
%define R_XMM13     616
%define R_XMM14     632
%define R_XMM15     648

%define R_MSR_EFER          664
%define R_MSR_STAR          672
%define R_MSR_LSTAR         680
%define R_MSR_CSTAR         688
%define R_MSR_SFMASK        696
%define R_MSR_KERNEL_GS     704
%define R_MSR_APIC_BASE     712

%define R_TLB_BASE          720
%define R_TLB_ENTRIES       728

%define R_TSS_RSP0  0x118
%define R_TSS_RSP1  0x120
%define R_TSS_RSP2  0x128

%define R_TSS_IST1  0x130
%define R_TSS_IST2  0x138
%define R_TSS_IST3  0x140
%define R_TSS_IST4  0x148
%define R_TSS_IST5  0x150
%define R_TSS_IST6  0x158
%define R_TSS_IST7  0x160

%define REGS_SIZE           792

; ============================================================
; Kernel state used by _cs_asm_get_regs
; ============================================================

tss_rsp0:
    dq 0

tss_rsp1:
    dq 0

tss_rsp2:
    dq 0

tlb_base:
    dq 0

tlb_entries:
    dq 0

; ============================================================
; Register capture
; ============================================================

_cs_asm_get_regs:

    ; --------------------------------------------------------
    ; Preserve destination pointer
    ; --------------------------------------------------------

    mov r10, rcx

    ; --------------------------------------------------------
    ; GPRs
    ; --------------------------------------------------------

    mov [r10 + R_RAX], rax
    mov [r10 + R_RBX], rbx
    mov [r10 + R_RCX], rcx
    mov [r10 + R_RDX], rdx
    mov [r10 + R_RSI], rsi
    mov [r10 + R_RDI], rdi
    mov [r10 + R_RBP], rbp
    mov [r10 + R_RSP], rsp

    mov [r10 + R_R8],  r8
    mov [r10 + R_R9],  r9
    mov [r10 + R_R10], r10
    mov [r10 + R_R11], r11
    mov [r10 + R_R12], r12
    mov [r10 + R_R13], r13
    mov [r10 + R_R14], r14
    mov [r10 + R_R15], r15


    ; ========================================================
    ; RIP
    ; ========================================================

    mov rax, [rsp]
    mov [r10 + R_RIP], rax


    ; ========================================================
    ; RFLAGS
    ; ========================================================

    pushfq
    pop rax
    mov [r10 + R_RFLAGS], rax


    ; ========================================================
    ; Segment registers
    ; ========================================================

    xor eax, eax
    mov ax, cs
    mov [r10 + R_CS], rax

    xor eax, eax
    mov ax, ds
    mov [r10 + R_DS], rax

    xor eax, eax
    mov ax, es
    mov [r10 + R_ES], rax

    xor eax, eax
    mov ax, ss
    mov [r10 + R_SS], rax

    xor eax, eax
    mov ax, fs
    mov [r10 + R_FS], rax

    xor eax, eax
    mov ax, gs
    mov [r10 + R_GS], rax


    ; ========================================================
    ; Control registers
    ; ========================================================

    mov rax, cr0
    mov [r10 + R_CR0], rax

    mov rax, cr2
    mov [r10 + R_CR2], rax

    mov rax, cr3
    mov [r10 + R_CR3], rax

    mov rax, cr4
    mov [r10 + R_CR4], rax

    mov rax, cr8
    mov [r10 + R_CR8], rax


    ; ========================================================
    ; Debug registers
    ; ========================================================

    mov rax, dr0
    mov [r10 + R_DR0], rax

    mov rax, dr1
    mov [r10 + R_DR1], rax

    mov rax, dr2
    mov [r10 + R_DR2], rax

    mov rax, dr3
    mov [r10 + R_DR3], rax

    mov rax, dr6
    mov [r10 + R_DR6], rax

    mov rax, dr7
    mov [r10 + R_DR7], rax


    ; ========================================================
    ; TSS RSP0 / RSP1 / RSP2
    ; ========================================================

    mov rax, [rel tss_rsp0]
    mov [r10 + R_TSS_RSP0], rax

    mov rax, [rel tss_rsp1]
    mov [r10 + R_TSS_RSP1], rax

    mov rax, [rel tss_rsp2]
    mov [r10 + R_TSS_RSP2], rax


    ; ========================================================
    ; FXSAVE
    ; ========================================================

    sub rsp, 512
    and rsp, -16

    fxsave64 [rsp]


    ; ========================================================
    ; x87 state
    ; ========================================================

    movzx eax, word [rsp + 0]
    mov [r10 + R_FPU_CW], ax

    movzx eax, word [rsp + 2]
    mov [r10 + R_FPU_SW], ax

    movzx eax, word [rsp + 4]
    mov [r10 + R_FPU_TW], ax

    movzx eax, word [rsp + 6]
    mov [r10 + R_FPU_OPCODE], ax

    mov rax, [rsp + 8]
    mov [r10 + R_FPU_IP], rax

    mov rax, [rsp + 16]
    mov [r10 + R_FPU_DP], rax

    movzx eax, word [rsp + 24]
    mov [r10 + R_FPU_CS], ax

    movzx eax, word [rsp + 26]
    mov [r10 + R_FPU_DS], ax


    ; ========================================================
    ; MMX
    ; ========================================================

    mov rax, [rsp + 32]
    mov [r10 + R_MMX0], rax

    mov rax, [rsp + 40]
    mov [r10 + R_MMX1], rax

    mov rax, [rsp + 48]
    mov [r10 + R_MMX2], rax

    mov rax, [rsp + 56]
    mov [r10 + R_MMX3], rax

    mov rax, [rsp + 64]
    mov [r10 + R_MMX4], rax

    mov rax, [rsp + 72]
    mov [r10 + R_MMX5], rax

    mov rax, [rsp + 80]
    mov [r10 + R_MMX6], rax

    mov rax, [rsp + 88]
    mov [r10 + R_MMX7], rax


    ; ========================================================
    ; MXCSR
    ; ========================================================

    mov eax, [rsp + 24]
    mov [r10 + R_MXCSR], eax

    mov eax, [rsp + 28]
    mov [r10 + R_MXCSR_MASK], eax


    ; ========================================================
    ; XMM0-XMM15
    ; ========================================================

    movdqu [r10 + R_XMM0],  xmm0
    movdqu [r10 + R_XMM1],  xmm1
    movdqu [r10 + R_XMM2],  xmm2
    movdqu [r10 + R_XMM3],  xmm3
    movdqu [r10 + R_XMM4],  xmm4
    movdqu [r10 + R_XMM5],  xmm5
    movdqu [r10 + R_XMM6],  xmm6
    movdqu [r10 + R_XMM7],  xmm7
    movdqu [r10 + R_XMM8],  xmm8
    movdqu [r10 + R_XMM9],  xmm9
    movdqu [r10 + R_XMM10], xmm10
    movdqu [r10 + R_XMM11], xmm11
    movdqu [r10 + R_XMM12], xmm12
    movdqu [r10 + R_XMM13], xmm13
    movdqu [r10 + R_XMM14], xmm14
    movdqu [r10 + R_XMM15], xmm15


    ; ========================================================
    ; MSRs
    ; ========================================================

    mov ecx, 0C0000080h
    rdmsr
    shl rdx, 32
    or rax, rdx
    mov [r10 + R_MSR_EFER], rax

    mov ecx, 0C0000081h
    rdmsr
    shl rdx, 32
    or rax, rdx
    mov [r10 + R_MSR_STAR], rax

    mov ecx, 0C0000082h
    rdmsr
    shl rdx, 32
    or rax, rdx
    mov [r10 + R_MSR_LSTAR], rax

    mov ecx, 0C0000083h
    rdmsr
    shl rdx, 32
    or rax, rdx
    mov [r10 + R_MSR_CSTAR], rax

    mov ecx, 0C0000084h
    rdmsr
    shl rdx, 32
    or rax, rdx
    mov [r10 + R_MSR_SFMASK], rax

    mov ecx, 0C0000102h
    rdmsr
    shl rdx, 32
    or rax, rdx
    mov [r10 + R_MSR_KERNEL_GS], rax

    mov ecx, 0000001Bh
    rdmsr
    shl rdx, 32
    or rax, rdx
    mov [r10 + R_MSR_APIC_BASE], rax

    ; ========================================================
    ; TSS RSP0-RSP2
    ; ========================================================

    mov rax, [rel tss_rsp0]
    mov [r10 + R_TSS_RSP0], rax

    mov rax, [rel tss_rsp1]
    mov [r10 + R_TSS_RSP1], rax

    mov rax, [rel tss_rsp2]
    mov [r10 + R_TSS_RSP2], rax

    ; ========================================================
    ; TSS IST1-IST7
    ; ========================================================

    mov rax, [rel tss_ist1]
    mov [r10 + R_TSS_IST1], rax

    mov rax, [rel tss_ist2]
    mov [r10 + R_TSS_IST2], rax

    mov rax, [rel tss_ist3]
    mov [r10 + R_TSS_IST3], rax

    mov rax, [rel tss_ist4]
    mov [r10 + R_TSS_IST4], rax

    mov rax, [rel tss_ist5]
    mov [r10 + R_TSS_IST5], rax

    mov rax, [rel tss_ist6]
    mov [r10 + R_TSS_IST6], rax

    mov rax, [rel tss_ist7]
    mov [r10 + R_TSS_IST7], rax

    ; ========================================================
    ; Restore original stack
    ; ========================================================

    mov rsp, [r10 + R_RSP]

    ret
