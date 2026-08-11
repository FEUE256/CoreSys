section .data

    ; This is the FÈUE CoreSyS offical IDT list 00-1F (Intel Fork)
    global _cs_asm_irq_de    ; 00 (0)  #DE Divide Error
    global _cs_asm_irq_db    ; 01 (1)  #DB Debug Exception
    global _cs_asm_irq_nmi   ; 02 (2)  NMI Interrupt
    global _cs_asm_irq_bp    ; 03 (3)  #BP Breakpoint
    global _cs_asm_irq_of    ; 04 (4)  #OF Overflow
    global _cs_asm_irq_br    ; 05 (5)  #BR Bound Range Exceeded
    global _cs_asm_irq_ud    ; 06 (6)  #UD Invalid Opcode
    global _cs_asm_irq_nm    ; 07 (7)  #NM Device Not Available
    global _cs_asm_irq_df    ; 08 (8)  #DF Double Fault
    global _cs_asm_irq_cs    ; 09 (9)  Coprocessor Segment Overrun (#CS in CoreSyS)
    global _cs_asm_irq_ts    ; 0A (10) #TS Invalid TSS
    global _cs_asm_irq_np    ; 0B (11) #NP Segment Not Present
    global _cs_asm_irq_ss    ; 0C (12) #SS Stack-Segment Fault
    global _cs_asm_irq_gp    ; 0D (13) #GP General Protection Fault
    global _cs_asm_irq_pf    ; 0E (14) #PF Page Fault
    global _cs_asm_irq_rs    ; 0F (15) #RS Reserved
    global _cs_asm_irq_mf    ; 10 (16) #MF x87 Floating-Point Error
    global _cs_asm_irq_ac    ; 11 (17) #AC Alignment Check
    global _cs_asm_irq_mc    ; 12 (18) #MC Machine Check
    global _cs_asm_irq_xm    ; 13 (19) #XM SIMD Floating-Point Exception
    global _cs_asm_irq_ve    ; 14 (20) #VE Virtualization Exception
    global _cs_asm_irq_cp    ; 15 (21) #CP Control Protection Exception
    global _cs_asm_irq_16    ; 16 (22) Reserved
    global _cs_asm_irq_17    ; 17 (23) Reserved
    global _cs_asm_irq_18    ; 18 (24) Reserved
    global _cs_asm_irq_19    ; 19 (25) Reserved
    global _cs_asm_irq_1A    ; 1A (26) Reserved
    global _cs_asm_irq_1B    ; 1B (27) Reserved
    global _cs_asm_irq_hv    ; 1C (28) #HV Hypervisor Injection Exception
    global _cs_asm_irq_vc    ; 1D (29) #VC VMM Communication Exception
    global _cs_asm_irq_sx    ; 1E (30) #SX Security Exception
    global _cs_asm_irq_1F    ; 1F (31) Reserved

section .text

    _cs_asm_irq_de: ; 00
        int 0x00
        ret

    _cs_asm_irq_db: ; 01
        int 0x01
        ret

    _cs_asm_irq_nmi: ; 02
        int 0x02
        ret

    _cs_asm_irq_bp: ; 03
        int 0x03
        ret

    _cs_asm_irq_of: ; 04
        int 0x04
        ret

    _cs_asm_irq_br: ; 05
        int 0x05
        ret

    _cs_asm_irq_ud: ; 06
        int 0x06
        ret

    _cs_asm_irq_nm: ; 07
        int 0x07
        ret

    _cs_asm_irq_df: ; 08
        int 0x08
        ret

    _cs_asm_irq_cs: ; 09
        int 0x09
        ret

    _cs_asm_irq_ts: ; 0A
        int 0x0A
        ret

    _cs_asm_irq_np: ; 0B
        int 0x0B
        ret

    _cs_asm_irq_ss: ; 0C
        int 0x0C
        ret

    _cs_asm_irq_gp: ; 0D
        int 0x0D
        ret

    _cs_asm_irq_pf: ; 0E
        int 0x0E
        ret

    _cs_asm_irq_rs: ; 0F
        int 0x0F
        ret

    _cs_asm_irq_mf: ; 10
        int 0x10
        ret

    _cs_asm_irq_ac: ; 11
        int 0x11
        ret

    _cs_asm_irq_mc: ; 12
        int 0x12
        ret

    _cs_asm_irq_xm: ; 13
        int 0x13
        ret

    _cs_asm_irq_ve: ; 14
        int 0x14
        ret

    _cs_asm_irq_cp: ; 15
        int 0x15
        ret

    _cs_asm_irq_16: ; 16
        int 0x16
        ret

    _cs_asm_irq_17: ; 17
        int 0x17
        ret

    _cs_asm_irq_18: ; 18
        int 0x18
        ret

    _cs_asm_irq_19: ; 19
        int 0x19
        ret

    _cs_asm_irq_1A: ; 1A
        int 0x1A
        ret

    _cs_asm_irq_1B: ; 1B
        int 0x1B
        ret

    _cs_asm_irq_hv: ; 1C
        int 0x1C
        ret

    _cs_asm_irq_vc: ; 1D
        int 0x1D
        ret

    _cs_asm_irq_sx: ; 1E
        int 0x1E
        ret

    _cs_asm_irq_1F: ; 1F
        int 0x1F
        ret