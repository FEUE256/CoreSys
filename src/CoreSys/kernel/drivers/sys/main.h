#pragma once 

#include <drivers/syscalls/main.h>

// Kernel syscall entry (extern)
uint64_t syscall(syscall_frame_t* frame);

// Syscall wrappers
char sys_read(void);
void sys_write(const char* s);
void sys_shutdown(void);
void sys_reboot(void);
void sys_init(void);
void sys_deinit(void);
void sys_clear(void);
void sys_reinit(void);
void sys_halt(void);
void sys_sf(const char* s);

#define SYS_READ     1
#define SYS_WRITE    2
#define SYS_SHUTDOWN 3
#define SYS_REBOOT   4
#define SYS_INIT     5
#define SYS_DEINIT   6
#define SYS_CLEAR    7
#define SYS_REINIT   8
#define SYS_HALT     9
#define SYS_SF       10

static inline uint64_t do_syscall(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx) {
    syscall_frame_t frame;
    frame.rax = rax;
    frame.rdi = rdi;
    frame.rsi = rsi;
    frame.rdx = rdx;
    return syscall(&frame);
}

char sys_read(void) {
    return (char)do_syscall(SYS_READ, 0, 0, 0);
}

void sys_write(const char* s) {
    do_syscall(SYS_WRITE, (uint64_t)s, 0, 0);
}

void sys_shutdown(void) {
    do_syscall(SYS_SHUTDOWN, 0, 0, 0);
}

void sys_reboot(void) {
    do_syscall(SYS_REBOOT, 0, 0, 0);
}

void sys_init(void) {
    do_syscall(SYS_INIT, 0, 0, 0);
}

void sys_deinit(void) {
    do_syscall(SYS_DEINIT, 0, 0, 0);
}

void sys_clear(void) {
    do_syscall(SYS_CLEAR, 0, 0, 0);
}

void sys_reinit(void) {
    do_syscall(SYS_REINIT, 0, 0, 0);
}

void sys_halt(void) {
    do_syscall(SYS_HALT, 0, 0, 0);
}

void sys_sf(const char* s) {
    do_syscall(SYS_SF, (uint64_t)s, 0, 0);
}
