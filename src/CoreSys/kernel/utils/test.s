BITS 64

section .text
global _start

_start:
    mov dx, 0x3F8

    mov al, 'C'
    out dx, al

    mov al, 'O'
    out dx, al

    mov al, 'P'
    out dx, al

    mov al, 'F'
    out dx, al

    mov al, 'S'
    out dx, al

    mov al, ' '
    out dx, al

    mov al, 'f'
    out dx, al

    mov al, 'i'
    out dx, al

    mov al, 'l'
    out dx, al

    mov al, 'e'
    out dx, al

    mov al, ' '
    out dx, al

    mov al, 'e'
    out dx, al

    mov al, 'x'
    out dx, al

    mov al, 'e'
    out dx, al

    mov al, 'c'
    out dx, al

    mov al, 'u'
    out dx, al

    mov al, 't'
    out dx, al

    mov al, 'i'
    out dx, al

    mov al, 'o'
    out dx, al

    mov al, 'n'
    out dx, al

    mov al, ' '
    out dx, al

    mov al, 'w'
    out dx, al

    mov al, 'o'
    out dx, al

    mov al, 'r'
    out dx, al

    mov al, 'k'
    out dx, al

    mov al, 's'
    out dx, al

    mov al, '!'
    out dx, al

    mov al, 10
    out dx, al

    pop rax
    jmp rax
    