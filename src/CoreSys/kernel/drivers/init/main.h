#pragma once

#include <stdint.h>             // Standard integer types
#include <drivers/serial/main.h>   // Serial port functions
#include <drivers/halt/main.h>       // Halt function
#include <drivers/sf/main.h>         // System Failure functions
#include <drivers/log/main.h>        // Logging functions
#include <drivers/page/main.h>       // Paging functions
#include <drivers/task/main.h>       // Task management functions
#include <drivers/cfs/main.h>        // CoreSys Filesystem (cfs)
#include <drivers/pci/main.h>        // PCI
#include <kernel/mem.h>
#include <cs.h>

CS_CORE core = {0};

// bpkg includes all drivers that you can init and CS_CORE

void init(cs_task* self) {
    (void)self; // Unused parameter

    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    cs_task init_serial_task = {
        .name = "Serial initialization Task",
        .source_header = "drivers/serial/main.h",
        .entry = initSerial
    };
    task_run(&init_serial_task); // init Serial Drivers
    if (debug != 2) { k_log("Serial port initialized successfully."); }

    a_char_print('1');

    cs_task init_cfs_task = {
        .name = "CFS initialization Task",
        .source_header = "drivers/cfs/main.h",
        .entry = cfs_init
    };
    task_run(&init_cfs_task); // init CFS
    if (debug != 2) { k_log("CFS initialized successfully."); }

    cs_task init_ahci_task = {
        .name = "AHCI initialization Task",
        .source_header = "drivers/pci/main.h",
        .entry = ahci_init
    };
    task_run(&init_ahci_task); // init AHCI
    if (debug != 2) { k_log("AHCI initialized successfully."); }

    cs_task init_pci_task = {
        .name = "PCI iGPU/GPU initialization Task",
        .source_header = "drivers/pci/main.h",
        .entry = pci_init
    };
    task_run(&init_pci_task); // init PCI
    if (debug != 2) { k_log("PCI iGPU/GPU initialized successfully."); }

    // For extra safty
    cs_init(&core);
    if (debug != 2) { k_log("CS CORE initialized successfully."); }

    if (debug != 2) { k_log("All drivers and utils in the bpkg (boot package) initialized successfully."); }
}

void deinit(cs_task* self) {
    (void)self; // Unused parameter

    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    if (debug != 2) { k_log("All drivers in the bpkg (boot package) may deinitialized successfully."); }

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

    cs_task deinit_ahci_task = {
        .name = "AHCI deinitialization Task",
        .source_header = "drivers/ahci/main.h",
        .entry = ahci_deinit
    };
    task_run(&deinit_ahci_task); // deinit AHCI

    cs_task deinit_pci_task = {
        .name = "PCI iGPU/GPU deinitialization Task",
        .source_header = "drivers/pci/main.h",
        .entry = pci_deinit
    };
    task_run(&deinit_pci_task); // deinit PCI

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
