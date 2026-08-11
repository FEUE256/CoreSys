; ====================
; KERNEL / ASM / OS
; CoreSys
; ====================

; Two thing: do not use hal_* or sys_* from C and everything is already initailized

extern kaprint ; in lib.s
extern init_d  ; in drivers/init/main.h

%define OFFSET_XMM0   280
%define OFFSET_XMM1   296
%define OFFSET_XMM2   312
%define OFFSET_XMM3   328
%define OFFSET_XMM4   344
%define OFFSET_XMM5   360
%define OFFSET_XMM6   376
%define OFFSET_XMM7   392
%define OFFSET_XMM8   408
%define OFFSET_XMM9   424
%define OFFSET_XMM10  440
%define OFFSET_XMM11  456
%define OFFSET_XMM12  472
%define OFFSET_XMM13  488
%define OFFSET_XMM14  504
%define OFFSET_XMM15  520

section .bss
    align 16

    stack_bottom:
        resb 0x100000      ; 1 MiB stack
    stack_top:

section .data
    global _cs_asm_init_main ; Init main globe
    global _cs_asm_stack_main ; Stack main
    global _cs_asm_get_regs ; Get regs

section .text
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

        ; newline (serial)
        mov al, 13
        call kaprint

        mov al, 10
        call kaprint

        ret

    _cs_asm_init_main:
        call init_print
        ret

    _cs_asm_get_regs:
        ; Win64 ABI:
        ; RCX = regs_t *regs

        ; General purpose registers
        mov [rcx + 0],   rax
        mov [rcx + 8],   rbx
        mov [rcx + 16],  rcx
        mov [rcx + 24],  rdx
        mov [rcx + 32],  rsi
        mov [rcx + 40],  rdi
        mov [rcx + 48],  rbp
        mov [rcx + 56],  rsp
        mov [rcx + 64],  r8
        mov [rcx + 72],  r9
        mov [rcx + 80],  r10
        mov [rcx + 88],  r11
        mov [rcx + 96],  r12
        mov [rcx + 104], r13
        mov [rcx + 112], r14
        mov [rcx + 120], r15

        ; Instruction state
        mov rax, [rsp]
        mov [rcx + 128], rax

        pushfq
        pop rax
        mov [rcx + 136], rax        ; RFLAGS

        ; Segment registers
        mov rax, cs
        mov [rcx + 144], rax
        mov rax, ds
        mov [rcx + 152], rax
        mov rax, es
        mov [rcx + 160], rax
        mov rax, ss
        mov [rcx + 168], rax
        mov rax, fs
        mov [rcx + 176], rax
        mov rax, gs
        mov [rcx + 184], rax

        ; Control registers
        mov rax, cr0
        mov [rcx + 192], rax

        mov rax, cr2
        mov [rcx + 200], rax

        mov rax, cr3
        mov [rcx + 208], rax

        mov rax, cr4
        mov [rcx + 216], rax

        mov rax, cr8
        mov [rcx + 224], rax

        ; Debug registers
        mov rax, dr0
        mov [rcx + 232], rax

        mov rax, dr1
        mov [rcx + 240], rax

        mov rax, dr2
        mov [rcx + 248], rax

        mov rax, dr3
        mov [rcx + 256], rax

        mov rax, dr6
        mov [rcx + 264], rax

        mov rax, dr7
        mov [rcx + 272], rax

        movdqu [rcx + OFFSET_XMM0], xmm0
        movdqu [rcx + OFFSET_XMM1], xmm1
        movdqu [rcx + OFFSET_XMM2], xmm2
        movdqu [rcx + OFFSET_XMM3], xmm3
        movdqu [rcx + OFFSET_XMM4], xmm4
        movdqu [rcx + OFFSET_XMM5], xmm5
        movdqu [rcx + OFFSET_XMM6], xmm6
        movdqu [rcx + OFFSET_XMM7], xmm7

        movdqu [rcx + OFFSET_XMM8],  xmm8
        movdqu [rcx + OFFSET_XMM9],  xmm9
        movdqu [rcx + OFFSET_XMM10], xmm10
        movdqu [rcx + OFFSET_XMM11], xmm11
        movdqu [rcx + OFFSET_XMM12], xmm12
        movdqu [rcx + OFFSET_XMM13], xmm13
        movdqu [rcx + OFFSET_XMM14], xmm14
        movdqu [rcx + OFFSET_XMM15], xmm15

        .return:
            ret
            