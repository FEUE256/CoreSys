// --------------------------------
// FÈUE File CoreSys Kernel
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

#include <init/kargs.h>        // Kernel arguments structure
#include <stdint.h>             // Standard integer types
#include <drivers/serial/main.h>   // Serial port functions
#include <drivers/halt/main.h>       // Halt function
#include <drivers/sf/main.h>         // System Failure functions
#include <drivers/log/main.h>        // Logging functions
#include <drivers/tty/main.h>        // TTY Terminal
// Do not include API/CoreSys.h it will cause k_sf()

/*
Guide to add custom driver

Include drivers/your/driver/main.h

Create your driver in drivers/..../main.h

Use pragma once and then just C code for your driver and then just impiment a driver-brige in drivers/tty/cmd.h

*/

int kmain(kargs* args) {

    k_log("CoreSys UEFI Kernel has been booted successfully.");

    initSerial(); // Init Serial
    k_log("Serial port initialized successfully.");

    k_log("FÈUE CoreSys Kernel Terminal");

    tty_loop(args);

    k_log("Kernel started");

    hlt();
    return 0; // dead code
}
