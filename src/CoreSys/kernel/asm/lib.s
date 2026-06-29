section .data  
    global kaprint ; for main.s ONLY so extern in asm/main.h
    global exit ; for main.s ONLY so extern in asm/main.h

section .text
    kaprint:
        mov dx, 0x3F8
        out dx, al
        ret ; Do not use call exit becasue it casues undefined things
