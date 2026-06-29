#pragma once

#include <mod/status.h>
#include <drivers/serial/main.h>
#include <drivers/halt/main.h>       // Halt function
#include <drivers/log/main.h>        // Logging functions
#include <stdarg.h>

void k_sf(const char *s) {
    serial_clear(NULL);

    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n\n");

    k_log("Sytem failure detected. Halting system to prevent damage.");
    k_log(s);

    serial_write("\r\n");
    serial_write("\r\nYour computer must shutdown now to prevent further damage.\r\n");
    serial_write("\r\nPress the power button to shutdown...");
    
    status = CS_KSF_OK;

    for (;;) {
        hlt(NULL);
    }

    // DEAD CODE |
    //           v
    
    status = CS_HALT_OK;
}

void k_sff(const char *fmt, ...)
{
    char buffer[1024];

    serial_clear(NULL);

    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n");
    serial_write("KERNEL SYSTEM FAILURE!!!\n\n");

    k_log("System failure detected. Halting system to prevent damage.");

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    k_log(buffer);

    serial_write("\r\n\r\nYour computer must shutdown now to prevent further damage.\r\n");
    serial_write("\r\nPress the power button to shutdown...");

    status = CS_KSF_OK;

    hlt(NULL);

    status = CS_HALT_OK;

    __builtin_unreachable();
}
