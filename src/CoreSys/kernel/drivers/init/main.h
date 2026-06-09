#pragma once

#include <stdint.h>             // Standard integer types
#include <drivers/serial/main.h>   // Serial port functions
#include <drivers/halt/main.h>       // Halt function
#include <drivers/sf/main.h>         // System Failure functions
#include <drivers/log/main.h>        // Logging functions
#include <drivers/page/main.h>       // Paging functions
#include <drivers/task/main.h>       // Task management functions
#include <drivers/cfs/main.h>        // CoreSys Filesystem (cfs)
#include <CoreSys.h>                 // CoreSys Main Header

CS_CORE core = {0};

void init(cs_task* self) {
    (void)self; // Unused parameter
    cs_task init_serial_task = {
        .name = "Serial initialization Task",
        .source_header = "drivers/serial/main.h",
        .entry = initSerial
    };
    task_run(&init_serial_task); // init Serial Drivers
    k_log("Serial port initialized successfully.");

    a_char_print('1');

    cs_task init_cfs_task = {
        .name = "CFS initialization Task",
        .source_header = "drivers/cfs/main.h",
        .entry = cfs_init
    };
    task_run(&init_cfs_task); // init CFS
    k_log("CFS initialized successfully.");

    cs_init(&core);
    k_log("CS CORE initialized successfully.");

    k_log("All drivers in the bpkg (boot package) initialized successfully.");
}

void deinit(cs_task* self) {
    (void)self; // Unused parameter
    k_log("All drivers in the bpkg (boot package) may deinitialized successfully.");

    cs_task deinit_serial_task = {
        .name = "Serial Deinitialization Task",
        .source_header = "drivers/serial/main.h",
        .entry = deinitSerial
    };
    task_run(&deinit_serial_task); // Deinit Serial Drivers

    cs_task deinit_cfs_task = {
        .name = "CFS Deinitialization Task",
        .source_header = "drivers/cfs/main.h",
        .entry = cfs_deinit
    };
    task_run(&deinit_cfs_task); // Deinit CFS

    cs_deinit(&core);
}

void reinit() {
    cs_task deinit_task = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task); // Deinit Drivers

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Reinit Drivers

}

void kreinit(cs_task* self) {
    (void)self; // Unused parameter
    cs_task deinit_task = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task); // Deinit Drivers

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Reinit Drivers

}
