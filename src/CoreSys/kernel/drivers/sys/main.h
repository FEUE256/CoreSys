#pragma once

#include <stdint.h>
#include <stddef.h>

#include <drivers/serial/main.h>
#include <drivers/ret/main.h>

// Syscall ABI frame
typedef struct {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
} syscall_frame_t;

// Kernel syscall entry (extern)
uint64_t syscall(syscall_frame_t* frame);

// Syscall wrappers
int sys_dev_null(void);
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

static inline uint64_t do_syscall(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx) {
    syscall_frame_t frame;
    frame.rax = rax;
    frame.rdi = rdi;
    frame.rsi = rsi;
    frame.rdx = rdx;
    return syscall(&frame);
}

int sys_dev_null(void) {
    return (int)do_syscall(0, 0, 0, 0);
}

char sys_read(void) {
    return (char)do_syscall(1, 0, 0, 0);
}

void sys_write(const char* s) {
    do_syscall(2, (uint64_t)s, 0, 0);
}

void sys_shutdown(void) {
    do_syscall(3, 0, 0, 0);
}

void sys_reboot(void) {
    do_syscall(4, 0, 0, 0);
}

void sys_init(void) {
    do_syscall(5, 0, 0, 0);
}

void sys_deinit(void) {
    do_syscall(6, 0, 0, 0);
}

void sys_clear(void) {
    do_syscall(7, 0, 0, 0);
}

void sys_reinit(void) {
    do_syscall(8, 0, 0, 0);
}

void sys_halt(void) {
    do_syscall(9, 0, 0, 0);
}

void sys_sf(const char* s) {
    do_syscall(10, (uint64_t)s, 0, 0);
}
