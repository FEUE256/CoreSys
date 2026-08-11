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
#include <drivers/cop/main.h>       // FS
#include <drivers/hw/ACPI/main.h>  // ACPI
#include <kernel/mem.h>
#include <asm/global.h>
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
        .entry_name = "initSerial",
        .entry = initSerial
    };
    g_core.task.task_run(&init_serial_task); // init Serial Drivers
    if (debug != 2) { k_log("Serial port initialized successfully."); }

    g_core.mem.pmm_init(
        0x100000,
        512 * 1024 * 1024
    );
    if (debug != 2) { k_log("Heap alloction initialized successfully."); }

    a_char_print('1');

    cs_task init_cfs_task = {
        .name = "CFS initialization Task",
        .source_header = "drivers/cfs/main.h",
        .entry_name = "cfs_init",
        .entry = cfs_init
    };
    g_core.task.task_run(&init_cfs_task); // init CFS
    if (debug != 2) { k_log("CFS initialized successfully."); }

    cs_task init_cop_task = {
        .name = "COP initialization Task",
        .source_header = "drivers/cop/main.h",
        .entry_name = "cop_init",
        .entry = cop_init
    };
    g_core.task.task_run(&init_cop_task); // init COP
    if (debug != 2) { k_log("COP initialized successfully."); }

    cs_task init_ahci_task = {
        .name = "AHCI initialization Task",
        .source_header = "drivers/pci/main.h",
        .entry_name = "ahci_init",
        .entry = ahci_init
    };
    g_core.task.task_run(&init_ahci_task); // init AHCI
    if (debug != 2) { k_log("AHCI initialized successfully."); }

    cs_task init_pci_task = {
        .name = "PCI/PCIe iGPU/GPU initialization Task",
        .source_header = "drivers/pci/main.h",
        .entry_name = "pci_init",
        .entry = pci_init
    };
    g_core.task.task_run(&init_pci_task); // init PCI
    if (debug != 2) { k_log("PCI iGPU/GPU initialized successfully."); }

    /*
        File stucture:
        /
        ├── sys
            ├── kernel
            |    ├── kernel.cfg
            ├── system
                ├── debug.cfg
                ├── boot
                    ├── bc.sctfi
    */

    cs_task init_fs_task = {
        .name = "FS initialization Task",
        .source_header = "drivers/cop/main.h",
        .entry_name = "fs_init",
        .entry = fs_init
    };
    g_core.task.task_run(&init_fs_task); // init FS
    if (debug != 2) { k_log("FS initialized successfully."); }

    cs_task init_acpi_task = {
        .name = "ACPI initialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_init",
        .entry = acpi_init
    };
    g_core.task.task_run(&init_acpi_task); // init ACPI
    if (debug != 2) { k_log("ACPI initialized successfully."); }

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
        .entry_name = "deinitSerial",
        .entry = deinitSerial
    };
    g_core.task.task_run(&deinit_serial_task); // Deinit Serial Drivers

    cs_task deinit_cfs_task = {
        .name = "CFS Deinitialization Task",
        .source_header = "drivers/cfs/main.h",
        .entry_name = "cfs_deinit",
        .entry = cfs_deinit
    };
    g_core.task.task_run(&deinit_cfs_task); // Deinit CFS

    cs_task deinit_ahci_task = {
        .name = "AHCI deinitialization Task",
        .source_header = "drivers/ahci/main.h",
        .entry_name = "ahci_deinit",
        .entry = ahci_deinit
    };
    g_core.task.task_run(&deinit_ahci_task); // deinit AHCI

    cs_task deinit_pci_task = {
        .name = "PCI iGPU/GPU deinitialization Task",
        .source_header = "drivers/pci/main.h",
        .entry_name = "pci_deinit",
        .entry = pci_deinit
    };
    g_core.task.task_run(&deinit_pci_task); // deinit PCI

    cs_task deinit_fs_task = {
        .name = "FS deinitialization Task",
        .source_header = "drivers/cop/main.h",
        .entry_name = "fs_deinit",
        .entry = fs_deinit
    };
    g_core.task.task_run(&deinit_fs_task); // deinit FS

    cs_task deinit_pmm_task = {
        .name = "PMM deinitialization Task",
        .source_header = "drivers/page/main.h",
        .entry_name = "pmm_deinit",
        .entry = pmm_deinit
    };
    g_core.task.task_run(&deinit_pmm_task); // deinit PMM

    // g_core.mem.pmm_deinit();

    cs_task deinit_cop_task = {
        .name = "COP deinitialization Task",
        .source_header = "drivers/cop/main.h",
        .entry_name = "cop_deinit",
        .entry = cop_deinit
    };
    g_core.task.task_run(&deinit_cop_task); // deinit COP

    // Dont deinit ACPI because its used to shutdown or reboot
    
    cs_deinit(&core);
}

void kreinit(cs_task* self) {
    (void)self; // Unused parameter
    cs_task deinit_task = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    g_core.task.task_run(&deinit_task); // Deinit Drivers

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    g_core.task.task_run(&init_task); // Reinit Drivers

}
