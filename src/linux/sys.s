.intel_syntax noprefix

.global write
.global read
.global fork
.global real_waitid
.global execve
.global _exit
.global kill_parent
.global getppid

kill_parent:
    # get parent PID
    mov rax, 39          # sys_getppid syscall number
    syscall               # returns parent PID in rax

    # rdi = PID, rsi = signal
    mov rdi, rax          # parent PID
    mov rsi, 9            # SIGKILL = 9

    mov rax, 62           # sys_kill syscall number
    syscall

    ret

write:
    mov rax, 1
    syscall
    ret

read:
    mov rax, 0
    syscall
    ret

fork:
    mov rax, 57
    syscall
    ret

# Waitid syscall
real_waitid:
    mov rax, 247
    mov r10, rcx
    syscall
    ret

# Execve syscall
execve:
    mov rax, 59
    syscall
    ret

# Exit syscall
_exit:
    mov rax, 60
    syscall
    # No need for ret
