; ====================
; KERNEL / ASM / OS
; CoreSys
; ====================

; Two thing: do not use hal_* or sys_* from C and everything is already initailized

extern kaprint ; in lib.s
extern init_d  ; in drivers/init/main.h

section .bss
    align 16

    stack_bottom:
        resb 0x100000      ; 1 MiB stack
    stack_top:

section .data
    global _cs_asm_init_main ; Init main globe
    global _cs_asm_test_main ; Test main globe
    global _cs_asm_stack_main ; Stack main

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

    _cs_asm_test_main:
        ud2
        ret
        