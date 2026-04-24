#pragma once

#include <drivers/serial/main.h>

void k_log(const char *s) {
    serial_write("[LOG] ");
    serial_write(s);
    serial_write("\n");
}
