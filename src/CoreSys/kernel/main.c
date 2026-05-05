// --------------------------------
// FÈUE File CoreSys Kernel
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

#include <init/kargs.h>        // Kernel arguments structure
#include <stdint.h>             // Standard integer types
#include <drivers/serial/main.h>   // Serial port functions
#include <drivers/halt/main.h>       // Halt function
#include <drivers/sf/main.h>         // System Failure functions
#include <drivers/log/main.h>        // Logging functions
#include <drivers/tty/main.h>        // TTY Terminal
#include <drivers/ACPI/main.h>       // ACPI functions
#include <drivers/page/main.h>       // Paging functions
#include <drivers/init/main.h>       // Initialization functions
#include <drivers/task/main.h>       // Task management functions

// Do not include API/CoreSys.h it will cause k_sf()

/*
Guide to add custom driver

Include drivers/your/driver/main.h

Create your driver in drivers/..../main.h

Use pragma once and then just C code for your driver and then just impiment a driver-brige in drivers/tty/cmd.h

*/

int kmain(kargs* args) {

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_log("CoreSys UEFI Kernel has been booted successfully.");

    kprint("Warning: The kernel is only for QEMU and may not work properly on real hardware. Use at your own risk.\n");
    k_log("FÈUE CoreSys Kernel Terminal");

    tty_loop(args);

    cs_task deinit_task = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt the system
    return 0; // dead code
}
