#pragma once

#include <drivers/serial/main.h>
#include <drivers/halt/main.h>       // Halt function
#include <drivers/log/main.h>        // Logging functions
#include <drivers/task/main.h>       // Task management functions

void k_sf(const char *s) {
    cs_task serial_clear_task = {
        .name = "Serial Clear Task",
        .source_header = "drivers/serial/main.h",
        .entry = serial_clear
    };
    task_run(&serial_clear_task); // Clear Serial Output

    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n\n");

    k_log("Sytem failure detected. Halting system to prevent damage.");
    k_log(s);

    serial_write("\r\n");
    serial_write("\r\nYour computer must shutdown now to prevent further damage.\r\n");
    serial_write("\r\nPress the power button to shutdown...");
    
    status = CS_KSF_OK;

    // ksf dont deinit

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry = hlt
    };
    task_run(&hlt_task); // Halt the system

    // DEAD CODE |
    //           v
    
    status = CS_HALT_OK;
}

