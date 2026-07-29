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

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

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

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

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

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI

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

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

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

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI

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

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

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

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}

void shutdown_hard(void)
{
    cs_task deinit_task1 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task1); // Deinit Drivers

    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_sf("Error: QEMU/Bochs shutdown command issued. If the system does not power off, please shut down manually.\n");

    cs_task deinit_task2 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task2); // Deinit Drivers

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}

void reboot_hard(void)
{
    cs_task deinit_task3 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task3); // Deinit Drivers

    outw(0x64, 0xFE);
    outb(0x64, 0xFE);

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers
    
    k_sf("Error: QEMU/Bochs reboot command issued. If the system does not reboot, please restart manually.\n");

    cs_task deinit_task4 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task4); // Deinit Drivers

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}


void kshutdown_hard(cs_task* self)
{
    (void)self; // Unused parameter
    cs_task deinit_task1 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task1); // Deinit Drivers

    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers

    k_sf("Error: QEMU/Bochs shutdown command issued. If the system does not power off, please shut down manually.\n");

    cs_task deinit_task2 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task2); // Deinit Drivers

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}

void kreboot_hard(cs_task* self)
{
    (void)self; // Unused parameter
    cs_task deinit_task3 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task3); // Deinit Drivers

    outw(0x64, 0xFE);
    outb(0x64, 0xFE);

    // Deinits ACPI
    cs_task deinit_acpi_task1 = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task1); // deinit ACPI

    cs_task init_task = {
        .name = "Initialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "init",
        .entry = init
    };

    task_run(&init_task); // Init Drivers
    
    k_sf("Error: QEMU/Bochs reboot command issued. If the system does not reboot, please restart manually.\n");

    cs_task deinit_task4 = {
        .name = "Deinitialization Task",
        .source_header = "drivers/init/main.h",
        .entry_name = "deinit",
        .entry = deinit
    };
    task_run(&deinit_task4); // Deinit Drivers

    // Deinits ACPI
    cs_task deinit_acpi_task = {
        .name = "ACPI deinitialization Task",
        .source_header = "drivers/hw/ACPI/main.h",
        .entry_name = "acpi_deinit",
        .entry = acpi_deinit
    };
    task_run(&deinit_acpi_task); // deinit ACPI

    cs_task hlt_task = {
        .name = "Halt Task",
        .source_header = "drivers/halt/main.h",
        .entry_name = "hlt",
        .entry = hlt
    };

    task_run(&hlt_task); // Halt Drivers
}

// No Cs_Task because this must work in an RKC Code
// (Thoose thats not with a _hard suffix is not designed for full RKC code because its uses ACPI thats deints )

void cs_shutdown(void)
{
    deinit(NULL);; // Deinit Drivers

    acpi_shutdown();

    acpi_deinit(NULL);;

    init(NULL); // Reinit Drivers

    k_sf("Error: ACPI shutdown command issued. If the system does not power off, please shut down manually.\n");

    deinit(NULL);; // Deinit Drivers

    acpi_deinit(NULL);;

    khlt(); // Halt CPU
}


void cs_reboot(void)
{
    deinit(NULL);; // Deinit Drivers

    acpi_reboot();

    acpi_deinit(NULL);;

    init(NULL); // Reinit Drivers

    k_sf("Error: ACPI reboot command issued. If the system does not reboot, please restart manually.\n");

    deinit(NULL);; // Deinit Drivers

    acpi_deinit(NULL);;

    khlt(); // Halt CPU
}


void cs_shutdown_hard(void)
{
    deinit(NULL);; // Deinit Drivers

    // QEMU / Bochs shutdown
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);

    acpi_deinit(NULL);;

    init(NULL); // Reinit Drivers

    k_sf("Error: QEMU/Bochs shutdown command issued. If the system does not power off, please shut down manually.\n");

    deinit(NULL);; // Deinit Drivers

    acpi_deinit(NULL);;

    khlt(); // Halt CPU
}


void cs_reboot_hard(void)
{
    deinit(NULL);; // Deinit Drivers

    // Keyboard controller reset
    outw(0x64, 0xFE);
    outb(0x64, 0xFE);

    acpi_deinit(NULL);;

    init(NULL); // Reinit Drivers

    k_sf("Error: QEMU/Bochs reboot command issued. If the system does not reboot, please restart manually.\n");

    deinit(NULL);; // Deinit Drivers

    acpi_deinit(NULL);;

    khlt(); // Halt CPU
}

// Designed for RKC

void k_cs_shutdown_hard(void)
{
    // QEMU / Bochs shutdown
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);

    init(NULL); // Reinit Drivers

    k_sf("Error: QEMU/Bochs shutdown command issued. If the system does not power off, please shut down manually.\n");

    deinit(NULL);; // Deinit Drivers

    acpi_deinit(NULL);;

    // No need for cs_deinit or init because cs_task isent used

    khlt(); // Halt CPU
}


void k_cs_reboot_hard(void)
{
    // Keyboard controller reset
    outw(0x64, 0xFE);
    outb(0x64, 0xFE);

    init(NULL); // Reinit Drivers

    k_sf("Error: QEMU/Bochs reboot command issued. If the system does not reboot, please restart manually.\n");

    deinit(NULL);; // Deinit Drivers

    acpi_deinit(NULL);;

    khlt(); // Halt CPU
}
