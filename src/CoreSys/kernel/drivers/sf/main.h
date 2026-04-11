#pragma once

#include <drivers/serial/main.h>
#include <drivers/halt/main.h>       // Halt function
#include <drivers/log/main.h>        // Logging functions

void k_sf(const char *s) {
    serial_clear();
    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n\n");

    k_log("Sytem failure detected. Halting system to prevent damage.");
    k_log(s);

    serial_write("\r\n");
    serial_write("\r\nYour computer must shutdown now to prevent further damage.\r\n");
    serial_write("\r\nPress the power button to shutdown...");
    hlt();
}

