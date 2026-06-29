// --------------------------------
// FÈUE File CoreSys Kernel
// Contains code of the project
// CyberBoot by FÈUE
// KERNEL / OS
// --------------------------------

// Includes / "Include"
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

    // Reads the debug int from RAM
    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    // Inits CS_CORE
    CS_CORE core;
    cs_init(&core);

    // Inits the CoreSys kernel
    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    // Runs the CoreSys kernel init task
    task_run(&init_task); // Init Drivers

    // Runs asm init if debug != is 2 (Not so importent init)
    if (debug != 2) { 
        _cs_asm_init_main(); // Makes sure ASM is working (in main.s)
    }

    // Makes the status that init has been OK
    status |= CS_INIT_OK;

    // Standard boot log
    if (debug != 2) { k_log("CoreSys UEFI Kernel has been booted successfully."); }

    // CoreSys.h BM/IM include assert 
    #ifdef CORESYS_H_INCLUDED
    #error CoreSys.h (BM/IM) is included
    assert("CoreSys.h (BM/IM) is included"); // This will not assert but it looks nice and will give compiler error because of multibale defintions
    #endif

    // Sstatus defined in globe.h
    sstatus = CS_SBOOT_OK;  // Bootloader OK
    sstatus = CS_SINIT_OK;  // Init layer OK
    sstatus = CS_SBOOT_OK;  // Kernel Boot OK
    sstatus = CS_SLOG_OK;   // Logging OK
    sstatus = CS_SDEBUG_OK; // Debug OK

    // Prints all debug text
    if (debug == 1) {
        // Prints what trace means
        k_log("TRACE = DEBUG/LOG/INTERNAL");

        // Prints register dump
        print_regs();

        // Prints the time at boot
        k_trace_n2("Time at Boot: ");
        get_time();

        // Prints the CMOS RegB
        k_trace_n2("CMOS RegB: ");
        get_regb();

        // Prints the debug int (it should be 1 if it prints)
        k_trace_n2("Debug Mode Int: ");
        kprint_int(debug);
        kprint("\n");

        // Prints the status and sstatus
        sprint();

        // Prints the PCI scan
        pci_scan();
        unumarr_t ahci_base =
            ahci_get_base(0, 31, 2);

        // Prints the AHCI base
        kprintf("AHCI base: %x\n", ahci_base);

        // Prints the ahci ports
        ahci_lp();

        // Prints the iGPU/GPU Info
        pci_igpu_info();

        // Tests pointers
        num_t x = 5;
        kprintf("Test addesses: %d @ %p\n", x, &x);

        // Test NULL pointers
        int *pN = NULL;
        kprintf("NULL Pointer %%p pN %p NULL Pointer %%d pN %d\n", pN, pN);
        kprintf("NULL Pointer %%p *pN %p NULL Pointer %%d *pN %d\n", *pN, *pN);
        kprintf("NULL Pointer %%p &pN %p NULL Pointer %%d &pN %d\n", &pN, &pN);

        // Test NULL
        kprintf("NULL %%p NULL %p NULL %%d NULL %d\n", NULL, NULL);

        // Prints debug texts
        k_warning("[TRACE] The kernel is only for QEMU and may not work properly on real hardware. If trying to use on real hardware can result in system damages. This OS is under active development, use at your own risk.");
        k_log("[TRACE] FÈUE CoreSys Kernel Terminal will be started in one secund. Subsystem status: Boot: OK, Init: OK, Kernel started: YES, Logging: ACTIVE, Debug Int: Available");
        k_log("[TRACE] Cheack 'sstatus' type CS_SUBSYS_STATUS_T for verdification and 'status' type CS_STATUS_T for general status");

        // Sets the status that the debug has been OK
        status |= CS_DEBUG_OK;
    } else {
        // If debug int = 0 print some normal text
        if (debug != 2) { 
            k_warning("The kernel is only for QEMU and may not work properly on real hardware. Use at your own risk.");
            k_log("FÈUE CoreSys Kernel Terminal");
        }
    }

    // Writes 0 to the arduino
    a_char_print('0');

    // Sets that TTY will run in a sec
    status |= CS_CTTY_OK; // TTY will run

    clear_screen(); // VGA
    vga_clear(); // VGA

    vga_println("No VGA support Serial only"); // VGA
    print_string("No VGA support Serial only!"); // VGA

    for (unsigned long long i = 0; i < 64000; i++) {
        framebuffer[i] = 0xFFFF0000;
        VGA_GRAPHICS[i] = 4; // red palette index
    }
    


    // Sets up tty_task task
    cs_task tty_task = {
        .name = "TTY Task",
        .source_header = "drivers/tty/*.h",
        .entry = tty_loop
    };

    // Runs the tty task
    task_run(&tty_task); // tty Drivers

    // Sets status to that TTY has ran
    status |= CS_TTY_OK; // TTY has been ran

    // Sets cs_return_value to core.sys.null() before CS_CORE has been deinit
    reg_t ret_t cs_return_value = core.sys.null();

    // Sets up deinit task
    cs_task deinit_task = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };

    // Runs the deinit task
    task_run(&deinit_task); // Deinit Drivers

    // Deinits CS_CORE
    cs_deinit(&core);

    // Sets that the CoreSys kernel has been deinit
    status |= CS_DEINIT_OK; // Deinit

    // Sets up the halt task
    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry = hlt
    };

    // Runs the Halt task
    task_run(&hlt_task); // Halt the system

    // DEAD CODE |
    //           v

    // Sets the status to the halt task has been run
    status |= CS_HALT_OK; // Halt

    // Return CS_SUCCESS via core.sys.null();
    return cs_return_value; // Can't use core.sys.null() because of cs_deinit has already happend (CS_SUCCESS)
}
