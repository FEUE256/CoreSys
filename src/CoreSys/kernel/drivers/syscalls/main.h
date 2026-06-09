#pragma once

#include <stdint.h>

#include <drivers/sys/main.h>

#include <drivers/serial/main.h>
#include <drivers/ACPI/main.h>
#include <drivers/init/main.h>
#include <drivers/task/main.h>
#include <drivers/halt/main.h>
#include <drivers/sf/main.h>
#include <drivers/ret/main.h>

// Syscall numbers (CoreSys ABI)
enum syscalls {
    SYS_DEV_NULL      = 0,
    SYS_READ          = 1,
    SYS_WRITE         = 2,
    SYS_SHUTDOWN      = 3,
    SYS_REBOOT        = 4,
    SYS_INIT          = 5,
    SYS_DEINIT        = 6,
    SYS_CLEAR         = 7,
    SYS_REINIT        = 8,
    SYS_HALT          = 9,
    SYS_SF            = 10,
};

// Driver functions (kernel layer)
int kret(void);
char kread(void);
void kprint(const char *s);
void kshutdown(cs_task* self); // Task STD
void kreboot(cs_task* self); // Task STD
void init(cs_task* self); // Task STD
void deinit(cs_task* self); // Task STD
void k_clear(cs_task* self); // Task STD
void kreinit(cs_task* self); // Task STD
void hlt(cs_task* self); // Task STD
void k_sf(const char *s);

// Syscall ABI type
typedef uint64_t (*syscall_fn)(
    uint64_t rdi,
    uint64_t rsi,
    uint64_t rdx
);

// Dispatcher
uint64_t syscall(syscall_frame_t* frame) {
    uint64_t id = frame->rax;

    if (id == SYS_DEV_NULL) {
        return (int)kret();
    }
    else if (id == SYS_READ) {
        return (uint64_t)kread();
    }
    else if (id == SYS_WRITE) {
        kprint((const char*)frame->rdi);
        return 0;
    }
    else if (id == SYS_SHUTDOWN) {
        cs_task kshutdown_task = {
            .name = "Kshutdown Task",
            .source_header = "drivers/ACPI/main.h",
            .entry = kshutdown
        };

        task_run(&kshutdown_task); // Kshutdown Drivers
        return 0;
    }
    else if (id == SYS_REBOOT) {
        cs_task kreboot_task = {
            .name = "Kreboot Task",
            .source_header = "drivers/ACPI/main.h",
            .entry = kreboot
        };

        task_run(&kreboot_task); // Kreboot Drivers
        return 0;
    }
    else if (id == SYS_INIT) {
        cs_task kinit_task = {
            .name = "Initialization Task",
            .source_header = "drivers/init/main.h",
            .entry = init
        };

        task_run(&kinit_task); // Init Drivers
        return 0;
    }
    else if (id == SYS_DEINIT) {
        cs_task kdeinit_task = {
            .name = "Deinitialization Task",
            .source_header = "drivers/init/main.h",
            .entry = deinit
        };
        task_run(&kdeinit_task); // Deinit Drivers
        return 0;
    }
    else if (id == SYS_CLEAR) {
        cs_task k_clear_task = {
            .name = "Clear Task",
            .source_header = "drivers/serial/main.h",
            .entry = k_clear
        };
        task_run(&k_clear_task); // Clear Drivers
        return 0;
    }
    else if (id == SYS_REINIT) {
        cs_task kreinit_task = {
            .name = "Reinitialization Task",
            .source_header = "drivers/init/main.h",
            .entry = kreinit
        };
        task_run(&kreinit_task); // Reinit Drivers
        return 0;
    }
    else if (id == SYS_HALT) {
        cs_task khlt_task = {
            .name = "Halt Task",
            .source_header = "drivers/halt/main.h",
            .entry = hlt
        };
        task_run(&khlt_task); // Halt Drivers
        return 0;
    }
    else if (id == SYS_SF) {
        k_sf((const char*)frame->rdi);
        return 0;
    } 
    else {
         __asm__ volatile (".word 0xFFFF"); // #UD for invalid syscall
    }   

    return (uint64_t)-1;
}
