#pragma once

#include <drivers/serial/main.h>

void k_log(const char *s) {
    serial_write("[LOG] ");
    serial_write(s);
    serial_write("\n");
}

void klog(const char *s) {
    serial_write("[KERNEL: LOG] ");
    serial_write(s);
    serial_write("\n");
}

void k_error(const char *s) {
    serial_write("[ERROR] ");
    serial_write(s);
    serial_write("\n");
}

void k_trace(const char *s) {
    serial_write("[TRACE] ");
    serial_write(s);
    serial_write("\n");
}

void k_warning(const char *s) {
    serial_write("[WARNING] ");
    serial_write(s);
    serial_write("\n");
}

void k_trace_n2(const char *s) {
    serial_write("[TRACE] ");
    serial_write(s);
}
