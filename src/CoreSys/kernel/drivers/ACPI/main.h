#pragma once

#include <drivers/serial/main.h>
#include <drivers/halt/main.h>
#include <drivers/init/main.h>
#include <drivers/task/main.h>

extern void init(cs_task* self);
extern void deinit(cs_task* self);

void shutdown(void)
{
    cs_task deinit_task1 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task1); // Deinit Drivers

    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_sf("Error: QEMU/Bochs shutdown command issued. If the system does not power off, please shut down manually.\n");

    cs_task deinit_task2 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task2); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}

void reboot(void)
{
    cs_task deinit_task3 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task3); // Deinit Drivers

    outw(0x64, 0xFE);
    outb(0x64, 0xFE);

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Init Drivers
    
    k_sf("Error: QEMU/Bochs reboot command issued. If the system does not reboot, please restart manually.\n");

    cs_task deinit_task4 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task4); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
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
        .entry = deinit
    };
    task_run(&deinit_task1); // Deinit Drivers

    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_sf("Error: QEMU/Bochs shutdown command issued. If the system does not power off, please shut down manually.\n");

    cs_task deinit_task2 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task2); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
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
        .entry = deinit
    };
    task_run(&deinit_task3); // Deinit Drivers

    outw(0x64, 0xFE);
    outb(0x64, 0xFE);

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry = init
    };

    task_run(&init_task); // Init Drivers
    
    k_sf("Error: QEMU/Bochs reboot command issued. If the system does not reboot, please restart manually.\n");

    cs_task deinit_task4 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry = deinit
    };
    task_run(&deinit_task4); // Deinit Drivers

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}
