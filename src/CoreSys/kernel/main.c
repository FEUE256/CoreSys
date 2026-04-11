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

int kmain(kargs* args) {
    initSerial(); // Init Serial
    k_log("Serial port initialized successfully.");

    k_log("FÈUE CoreSys Kernel Terminal");

    tty_loop(args);

    k_log("Kernel started");

    hlt();
    return 0; // dead code
}
