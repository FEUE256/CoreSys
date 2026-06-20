; CIOS FÈUE
; Part of the CoreSys ecosystem
; FIRMWARE

BITS 16

start:
    mov al, "H"
    call printf

    mov al, "i"
    call printf

    hlt

printf:
    out 0xE9, al
    ret
