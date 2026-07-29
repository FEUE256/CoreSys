#pragma once

#include <mod/globe.h>

void rkc_enter() {
    rkc_mode = 1;
}

void rkc_ret() {
    rkc_mode = 0;
}
