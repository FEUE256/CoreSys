#pragma once

// General status
typedef enum {
    CS_BOOTL_OK,         // Bootloader
    CS_INITL_OK,        // Init Layer
    CS_BOOT_OK,        // Kernel Boot
    CS_INIT_OK,       // Kernel Init
    CS_DEBUG_OK,     // Debug Mode print 
    CS_CTTY_OK,     // TTY will be running
    CS_TTY_OK,     // TTY Has been ran
    CS_DEINIT_OK, // Deinit 
    CS_HALT_OK,  // Halted
    CS_KSF_OK   // Kernel System Failure
} CS_STATUS_T;

// Subsystem Status
typedef enum {
    CS_SBOOT_OK,     // Boot 
    CS_SINIT_OK,    // Init
    CS_SKERNEL_OK, // Kernel 
    CS_SLOG_OK,   // Log 
    CS_SDEBUG_OK // Debug 
} CS_SUBSYS_STATUS_T;

// For both STATUS systems if eg status = CS_TTY_OK then all statuses before it will be OK exeption: CS_KSF_OK 
