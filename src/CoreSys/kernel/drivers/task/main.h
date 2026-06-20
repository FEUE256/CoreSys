#pragma once

// Task management system for CoreSys kernel drivers API

// If you are lazy set source_header to be NULL or empty string, but it's recommended to set it to the header file of the driver for better debugging and module origin tracking.

#include <mod/globe.h>

typedef struct cs_task {
    const char* name;              // logical task name
    const char* source_header;    // metadata only (debug / module origin)
    void (*entry)(struct cs_task* self); // actual executable function
} cs_task;

void task_run(cs_task* task) {
   if (tsk_ready == 0) {
        return; // Can't use core->sys.null() (bsc of header loop)
   } else {
    if (!task || !task->entry) return;
    task->entry(task);
   }
}
