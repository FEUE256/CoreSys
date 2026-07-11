#pragma once

#include <stdint.h>
#include <stddef.h>

#include <drivers/fscalls/main.h>

#include <drivers/serial/main.h>
#include <drivers/ret/main.h>

// Kernel fscall entry (extern)
uint64_t fscall(fscall_frame_t *frame);

// fscall wrappers
int fs_ret(int code);
bool fs_cop_init();
uint64_t fs_cop_deinit();
int fs_cop_mkdir(const char *path);
int fs_cop_create(const char *path);
int fs_cop_write(const char *path, const void *buffer, uint64_t size);
int fs_cop_read(const char *path, void *buffer, uint64_t size);
int fs_cop_delete(const char *path);
int fs_cop_append(const char *path, const void *buffer, uint64_t size);
uint64_t fs_cop_exec_file(const char *path);
uint64_t fs_fs_init();
uint64_t fs_fs_deinit();

static inline uint64_t do_fscall(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx) {
    fscall_frame_t frame;
    frame.rax = rax;
    frame.rdi = rdi;
    frame.rsi = rsi;
    frame.rdx = rdx;
    return fscall(&frame);
}

int fs_dev_null(int code) {
    return (int)do_fscall(FS_DEV_NULL, (uint64_t)code, 0, 0);
}

bool fs_cop_init(void) {
    return do_fscall(COP_INIT, 0, 0, 0);
}

uint64_t fs_cop_deinit(void) {
    return do_fscall(COP_DEINIT, 0, 0, 0);
}

int fs_cop_mkdir(const char *path) {
    return (int)do_fscall(COP_CREATE_DIR, (uint64_t)path, 0, 0);
}

int fs_cop_create(const char *path) {
    return (int)do_fscall(COP_CREATE_FILE, (uint64_t)path, 0, 0);
}

int fs_cop_write(const char *path, const void *buffer, uint64_t size) {
    return (int)do_fscall(
        COP_WRITE_FILE,
        (uint64_t)path,
        (uint64_t)buffer,
        size
    );
}

int fs_cop_read(const char *path, void *buffer, uint64_t size) {
    return (int)do_fscall(
        COP_READ_FILE,
        (uint64_t)path,
        (uint64_t)buffer,
        size
    );
}

int fs_cop_delete(const char *path) {
    return (int)do_fscall(COP_DELETE_FILE, (uint64_t)path, 0, 0);
}

int fs_cop_append(const char *path, const void *buffer, uint64_t size) {
    return (int)do_fscall(
        COP_APPEND_FILE,
        (uint64_t)path,
        (uint64_t)buffer,
        size
    );
}

uint64_t fs_cop_exec_file(const char *path) {
    return do_fscall(COP_EXEC_FILE, (uint64_t)path, 0, 0);
}

uint64_t fs_fs_init(void) {
    return do_fscall(COP_FS_INIT, 0, 0, 0);
}

uint64_t fs_fs_deinit(void) {
    return do_fscall(COP_FS_DEINIT, 0, 0, 0);
}
