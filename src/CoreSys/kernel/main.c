// ------------------------------------------
// FÈUE File CoreSys Kernel
// Contains code of the project
// CyberBoot by FÈUE
// KERNEL / OS
// Namespace io.feue.coresys.kernel.kmain
// ------------------------------------------

// Include
#include <CoreSys.h>                 // CoreSys Main Header

// Do not include API/CoreSys.h it will cause k_sf()

/*
Guide to add custom driver

Include drivers/your/driver/main.h
Create your driver in drivers/..../main.h
Use pragma once and then just C code for your driver and then just impiment a driver-brige in drivers/tty/cmd.h

Remember to add a driver include to CoreSys.h
*/

// ABI
// 4 spaces in a tab then at 5:th char the letter

#ifndef KERNEL
#error "This file must be compiled as kernel code"
#endif

ret_t kmain(void) {
    rkc_enter();

    // Status define in globe.h
    status |= CS_BOOTL_OK; // Bootloader OK
    status |= CS_INITL_OK; // Init layer OK
    status |= CS_BOOT_OK;  // Kernel Boot OK

    // Reads the debug int from RAM
    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum64_t debug = (num_t)(*slot);

    // Checking if KDI is 404 
    // This means the system must shutdown
    if (debug == 148) { // (404)
        k_cs_shutdown_hard(); // k_ means No deinit because the system hasent been inited
        khlt(); // Halts if failure
    }

    rkc_ret();

    // RKC Mode  ^
    //           |

    // Inits CS_CORE
    CS_CORE core;
    cs_init(&core);

    // Inits Global CS_CORE thats defined in globe.h
    cs_init(&g_core);

    // Inits the CoreSys kernel
    core.sys.init();

    cs_task boots_task = {
        .name = "Boot Sound Task",
        .source_header = "mod/sound.h",
        .entry_name = "boot_sound",
        .entry = boot_sound
    };
    core.task.task_run(&boots_task);

    // Runs asm init if debug != is 2 ("init" more like a test)
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
    assert_hard("CoreSys.h (BM/IM) is included"); // This will not assert but it looks nice and will give compiler error because of multi defintions from CoreSys.h (BM/IM)
    #endif

    // Sstatus defined in globe.h
    sstatus = CS_SBOOT_OK;  // Bootloader OK
    sstatus = CS_SINIT_OK;  // Init layer OK
    sstatus = CS_SBOOT_OK;  // Kernel Boot OK
    sstatus = CS_SLOG_OK;   // Logging OK
    sstatus = CS_SDEBUG_OK; // Debug OK

    // Prints all debug text
    if (debug == 1) {
        kprintf("Run pd in the terminal to access debug mode!\n");
        // Prints debug texts
        k_warning("[TRACE] The kernel is only for QEMU and may not work properly on real hardware. If trying to use on real hardware can result in system damages. This OS is under active development, use at your own risk.");
        k_log("[TRACE] FÈUE CoreSys Kernel Terminal will be started in one secund. Subsystem status: Boot: OK, Init: OK, Kernel started: YES, Logging: ACTIVE, Debug Int: Available");
        k_log("[TRACE] Cheack 'sstatus' type CS_SUBSYS_STATUS_T for verdification and 'status' type CS_STATUS_T for general status");
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

    cs_task vga_task = {
        .name = "VGA Task",
        .source_header = "drivers/vga/main.h",
        .entry_name = "print_vga",
        .entry = print_vga
    };
    core.task.task_run(&vga_task);

    // Runs TTY loop via CS_CORE
    core.hal.tty_loop();
    __builtin_unreachable();

    // Sets status to that TTY has ran
    status |= CS_TTY_OK; // TTY has been ran

    // Sets cs_return_value to core.sys.null() before CS_CORE has been deinit
    reg_t ret_t cs_return_value = core.task.null(1);

    // Runs deinit via CS_CORE
    core.sys.deinit();

    // Deinits CS_CORE
    cs_deinit(&core);

    // RKC Mode  |
    //           v

    rkc_enter();

    // Sets that the CoreSys kernel has been deinit
    status |= CS_DEINIT_OK; // Deinit

    // Shutdown Hard (Can't use ACPI and CS_TASK)
    k_cs_shutdown_hard();

    // Cant use core.sys.halt because cs_core is deinited
    // Cant use cs_task because its deinited
    // Can technicly use sys_halt/_hlt but thats not used in RKC
    // Halts because shutdown failed
    khlt();

    // Sets the status to the halt task has been run
    status |= CS_HALT_OK; // Halt

    // Why did halt fail, trying to #MC
    _cs_asm_irq_mc(); // If fail continue to RKC ret and enter MRKC

    rkc_ret();

    // MRKC Maximal Restriced Kernel C |
    //                                 v
    // MRKC does only return the kernel to init

    // Return CS_SUCCESS via core.sys.null();
    return cs_return_value; // Can't use core.sys.null() because of cs_deinit has already happend (CS_SUCCESS)
    __builtin_unreachable();
}
