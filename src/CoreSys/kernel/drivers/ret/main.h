#pragma once

#include <mod/status.h>
#include <drivers/serial/main.h>

int ret(int code) { // HAL
    return code;
}

int kret(void) { // SYS
    return CS_SUCCESS;
}

// Shout out for idé to NVMe driver (return -sc in nvme_poll_cq)
int fsret(int code) { // FS
    return -code;
}

int irqret(int code) { // IRQ
    return abs(code);
}

// Example core.fs.null(core.irq.null(status))