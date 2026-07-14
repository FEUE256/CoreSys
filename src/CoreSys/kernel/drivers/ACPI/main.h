#pragma once

#include <drivers/serial/main.h>
#include <drivers/halt/main.h>
#include <drivers/init/main.h>
#include <drivers/task/main.h>
#include <drivers/hw/ACPI/main.h>

extern void init(cs_task* self);
extern void deinit(cs_task* self);

void shutdown(void)
{
    cs_task deinit_task1 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task1); // Deinit Drivers

    acpi_shutdown();

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI


    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_sf("Error: ACPI shutdown command issued. If the system does not power off, please shut down manually.\n");

    cs_task deinit_task2 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task2); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}

void reboot(void)
{
    cs_task deinit_task3 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task3); // Deinit Drivers

    acpi_reboot();

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers
    
    k_sf("Error: ACPI reboot command issued. If the system does not reboot, please restart manually.\n");

    cs_task deinit_task4 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task4); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}


void kshutdown(cs_task* self)
{
    (void)self; // Unused parameter
    cs_task deinit_task1 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task1); // Deinit Drivers

    acpi_shutdown();

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_sf("Error: ACPI shutdown command issued. If the system does not power off, please shut down manually.\n");

    cs_task deinit_task2 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task2); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}

void kreboot(cs_task* self)
{
    (void)self; // Unused parameter
    cs_task deinit_task3 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task3); // Deinit Drivers

    acpi_reboot();

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers
    
    k_sf("Error: ACPI reboot command issued. If the system does not reboot, please restart manually.\n");

    cs_task deinit_task4 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task4); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}
