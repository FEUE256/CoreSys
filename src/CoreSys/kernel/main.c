// --------------------------------
// FÈUE File CoreSys Kernel
// Contains code of the project
// CyberBoot by FÈUE
// KERNEL / OS
// --------------------------------

#include <CoreSys.h>                 // CoreSys Main Header

// Do not include API/CoreSys.h it will cause k_sf()

/*
Guide to add custom driver

Include drivers/your/driver/main.h

Create your driver in drivers/..../main.h

Use pragma once and then just C code for your driver and then just impiment a driver-brige in drivers/tty/cmd.h

Remember to add a driver include to CoreSys.h

*/

int kmain() {
    CS_CORE core;
    cs_init(&core);

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_log("CoreSys UEFI Kernel has been booted successfully.");

    volatile uint8_t *slot = (volatile uint8_t*)KDI;
    int debug = (int)(*slot);

    if (debug == 1) {
        k_log("TRACE = DEBUG/LOG/INTERNAL");

        print_regs();

        k_trace_n2("Time at Boot: ");
        get_time();

        k_trace_n2("CMOS RegB: ");
        get_regb();

        k_trace_n2("Debug Mode Int: ");
        kprint_int(debug);
        kprint("\n");

        k_warning("[TRACE] The kernel is only for QEMU and may not work properly on real hardware. If trying to use on real hardware can result in system damages. This OS is under active development, use at your own risk.");
        k_log("[TRACE] FÈUE CoreSys Kernel Terminal will be started in one secund. Subsystem status: Boot: OK, Init: OK, Kernel started: YES, Logging: ACTIVE, Debug Int: Available");
    } else {
        k_warning("The kernel is only for QEMU and may not work properly on real hardware. Use at your own risk.");
        k_log("FÈUE CoreSys Kernel Terminal");
    }

    a_char_print('0');

    tty_loop(debug);

    int cs_return_value = core.sys.null();

    cs_task deinit_task = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task); // Deinit Drivers

    cs_deinit(&core);

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt the system

    // DEAD CODE |
    //           v

    return cs_return_value; // Can't use core.sys.null() because of cs_deinit has already happend
}
