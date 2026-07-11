#pragma once

#include <mod/status.h>

int ret(int code) {
    return code;
}

int kret(void) {
    return CS_SUCCESS;
}

// Shout out for idé to NVMe driver (return -sc in nvme_poll_cq)
int fsret(int code) {
    return -code;
}
