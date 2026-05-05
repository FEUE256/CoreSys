#pragma once

#include <drivers/task/main.h>       // Task management functions

void hlt(cs_task* self) {
    (void)self; // Unused parameter
    __asm__ volatile ("cli");
    for (;;)
        __asm__ volatile ("hlt");
}

void khlt(void) {
    __asm__ volatile ("cli");
    for (;;)
        __asm__ volatile ("hlt");
}
