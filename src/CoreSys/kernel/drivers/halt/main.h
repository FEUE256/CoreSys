#pragma once

void hlt(void) {
    __asm__ volatile ("cli");
    for (;;)
        __asm__ volatile ("hlt");
}
