#pragma once

#include <stdint.h>

#include <drivers/irq/main.h>
#include <drivers/cop/main.h>

// Fscalls numbers (CoreSys ABI)
enum copcalls {
    FS_DEV_NULL      = 0,
    COP_INIT         = 1,
    COP_DEINIT       = 2,
    COP_CREATE_DIR   = 3,
    COP_WRITE_FILE   = 4,
    COP_READ_FILE    = 5,
    COP_DELETE_FILE  = 6,
    COP_APPEND_FILE  = 7,
    COP_EXEC_FILE    = 8,
    COP_FS_INIT      = 9,
    COP_FS_DEINIT    = 10,
    COP_CREATE_FILE  = 11
};

// Fscalls ABI frame
typedef struct {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
} fscall_frame_t;

// Driver functions (kernel layer)
int fsret(int code);
void cop_init(cs_task *self);
void cop_deinit(cs_task *self);
bool cop_mkdir(const char *path);
bool cop_create(const char *path);
bool cop_write(const char *path, const void *buffer, uint64_t size);
bool cop_read(const char *path, void *buffer, uint64_t size);
bool cop_delete(const char *path);
bool cop_append(const char *path, const void *buffer, uint64_t size);
void cop_exec_file(const char *path);
void fs_init(cs_task *self);
void fs_deinit(cs_task *self);

// Dispatcher
uint64_t fscall(fscall_frame_t *frame)
{
    uint64_t id = frame->rax;

    if (id == FS_DEV_NULL) {
        return (uint64_t)fsret((int)frame->rdi);
    }
    else if (id == COP_INIT) {
        cs_task cop_init_task = {
            .name = "COP Init Task",
            .source_header = "drivers/cop/main.h",
            .entry_name = "cop_init",
            .entry = cop_init
        };
        task_run(&cop_init_task);

        return 0;
    }
    else if (id == COP_DEINIT) {
        cs_task cop_deinit_task = {
            .name = "COP Deinit Task",
            .source_header = "drivers/cop/main.h",
            .entry_name = "cop_deinit",
            .entry = cop_deinit
        };
        task_run(&cop_deinit_task);

        return 0;
    }
    else if (id == COP_CREATE_DIR) {
        return (uint64_t)cop_mkdir((const char*)frame->rdi);
    }
    else if (id == COP_WRITE_FILE) {
        return (uint64_t)cop_write(
            (const char*)frame->rdi,
            (const void*)frame->rsi,
            frame->rdx
        );
    }
    else if (id == COP_READ_FILE) {
        return (uint64_t)cop_read(
            (const char*)frame->rdi,
            (void*)frame->rsi,
            frame->rdx
        );
    }
    else if (id == COP_DELETE_FILE) {
        return (uint64_t)cop_delete((const char*)frame->rdi);
    }
    else if (id == COP_APPEND_FILE) {
        return (uint64_t)cop_append(
            (const char*)frame->rdi,
            (const void*)frame->rsi,
            frame->rdx
        );
    }
    else if (id == COP_EXEC_FILE) {
        cop_exec_file((const char*)frame->rdi);
        return 0;
    }
    else if (id == COP_FS_INIT) {
        cs_task fs_init_task = {
            .name = "FS Init Task",
            .source_header = "drivers/cop/main.h",
            .entry_name = "fs_init",
            .entry = fs_init
        };
        task_run(&fs_init_task);

        return 0;
    }
    else if (id == COP_FS_DEINIT) {
        cs_task fs_deinit_task = {
            .name = "FS Deinit Task",
            .source_header = "drivers/cop/main.h",
            .entry_name = "fs_deinit",
            .entry = fs_deinit
        };
        task_run(&fs_deinit_task);

        return 0;
    }
    else if (id == COP_CREATE_FILE) {
        return (uint64_t)cop_create((const char*)frame->rdi);
    }
    else {
        __asm__ volatile(".word 0xFFFF"); // #UD
        irq(CS_IRQ_UD);
    }

    return (uint64_t)-1;
}
