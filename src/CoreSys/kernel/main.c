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

ret_t kmain() {
    // Status define in globe.h
    status |= CS_BOOTL_OK; // Bootloader OK
    status |= CS_INITL_OK; // Init layer OK
    status |= CS_BOOT_OK;  // Kernel Boot OK

    CS_CORE core;
    cs_init(&core);

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    status |= CS_INIT_OK;

    k_log("CoreSys UEFI Kernel has been booted successfully.");

    #ifdef CORESYS_H_INCLUDED
    #error CoreSys.h (BM/IM) is included
    assert("CoreSys.h (BM/IM) is included"); // This will not assert but it looks nice and will give compiler error because of multibale defintions
    #endif

    volatile unum8_t *slot = (volatile unum8_t*)KDI;
    num_t debug = (num_t)(*slot);

    // Sstatus defined in globe.h
    sstatus = CS_SBOOT_OK;  // Bootloader OK
    sstatus = CS_SINIT_OK;  // Init layer OK
    sstatus = CS_SBOOT_OK;  // Kernel Boot OK
    sstatus = CS_SLOG_OK;   // Logging OK
    sstatus = CS_SDEBUG_OK; // Debug OK

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

        sprint();

        pci_scan();
        uintptr_t ahci_base =
            ahci_get_base(0, 31, 2);

        kprintf("AHCI base: %x\n", ahci_base);

        ahci_lp();

        pci_igpu_info();

        k_warning("[TRACE] The kernel is only for QEMU and may not work properly on real hardware. If trying to use on real hardware can result in system damages. This OS is under active development, use at your own risk.");
        k_log("[TRACE] FÈUE CoreSys Kernel Terminal will be started in one secund. Subsystem status: Boot: OK, Init: OK, Kernel started: YES, Logging: ACTIVE, Debug Int: Available");
        k_log("[TRACE] Cheack 'sstatus' type CS_SUBSYS_STATUS_T for verdification and 'status' type CS_STATUS_T for general status");

        status |= CS_DEBUG_OK;
    } else {
        k_warning("The kernel is only for QEMU and may not work properly on real hardware. Use at your own risk.");
        k_log("FÈUE CoreSys Kernel Terminal");
    }

    a_char_print('0');

    status |= CS_CTTY_OK; // TTY will run

    // for (int i = 0; i < 100; i++)
    // {
    //     framebuffer[i] = 0xFFFF0000;
    // }  // TOOD: Make it work mod/globe.h drivers/pci/main.h

    tty_loop(debug);

    status |= CS_TTY_OK; // TTY has been ran

    ret_t cs_return_value = core.sys.null();

    cs_task deinit_task = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task); // Deinit Drivers

    cs_deinit(&core);

    status |= CS_DEINIT_OK; // Deinit

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt the system

    // DEAD CODE |
    //           v

    status |= CS_HALT_OK; // Halt

    return cs_return_value; // Can't use core.sys.null() because of cs_deinit has already happend
}
