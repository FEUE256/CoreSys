#pragma once 

#include <drivers/serial/main.h>
#include <drivers/log/main.h>
#include <globe.h>
#include <status.h>

const char* cs_status_decode(CS_STATUS_T s)
{
    switch (s)
    {
        case CS_BOOTL_OK: return "BOOTL_OK";
        case CS_INITL_OK: return "INITL_OK";
        case CS_BOOT_OK:  return "BOOT_OK";
        case CS_INIT_OK:  return "INIT_OK";
        case CS_DEBUG_OK: return "DEBUG_OK";
        case CS_CTTY_OK:  return "CTTY_OK";
        case CS_TTY_OK:   return "TTY_OK";
        case CS_DEINIT_OK:return "DEINIT_OK";
        case CS_HALT_OK:  return "HALT_OK";
        case CS_KSF_OK:   return "KSF_OK";
        default:          return "UNKNOWN";
    }
}

const char* cs_sstatus_decode(CS_SUBSYS_STATUS_T s)
{
    switch (s)
    {
        case CS_SBOOT_OK:    return "SBOOT_OK";
        case CS_SINIT_OK:    return "SINIT_OK";
        case CS_SKERNEL_OK:  return "SKERNEL_OK";
        case CS_SLOG_OK:     return "SLOG_OK";
        case CS_SDEBUG_OK:   return "SDEBUG_OK";
        default:             return "UNKNOWN";
    }
}

void sprint() {
    k_trace_n2("Status: ");
    kprint(cs_status_decode(status));
    kprint("\n");

    k_trace_n2("Sstatus: ");
    kprint(cs_sstatus_decode(sstatus));
    kprint("\n");
}
