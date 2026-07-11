#pragma once

// Task management system for CoreSys kernel drivers API

// If you are lazy set source_header to be NULL or empty string, but it's recommended to set it to the header file of the driver for better debugging and module origin tracking.

#include <mod/globe.h>
#include <kernel/mem.h>

#include <drivers/serial/main.h>
extern int tsk_ready;

int kprintf(const char *fmt, ...);

typedef struct cs_task {
    const char* name;                       // logical task name
    const char* source_header;             // metadata only (debug / module origin)
    const char* entry_name;               // For debug purposes
    void (*entry)(struct cs_task* self); // actual executable function
} cs_task;

void task_run(cs_task* task)
{
    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    if (tsk_ready == 0)
    {
        if (debug != 2) { kprintf("[TASK] scheduler not ready\n"); }
        return;
    }

    if (!task)
    {
        if (debug != 2) { kprintf("[TASK] NULL task\n"); }
        return;
    }

    if (!task->entry)
    {
       if (debug != 2) {  kprintf("[TASK] NULL entry\n"); }
        return;
    }

    if (debug == 1) {
        kprintf("[TASK] A CS_TASK has been called with Entry Pointer: %p Entry Name: %s Source Header: %s CS_TASK name: %s\n", 
            task->entry, task->entry_name, task->source_header, task->name);
    }

    task->entry(task);
}
