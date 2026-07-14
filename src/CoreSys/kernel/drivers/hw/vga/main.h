// PCI/PCIe VGA Driver (Approved by FÈUE in the CoreSys kernel hardware drivers collation)
#pragma once

#include <drivers/serial/main.h>
#include <drivers/pci/main.h>
#include <drivers/sf/main.h>
#include <drivers/task/main.h>
#include <drivers/halt/main.h>

#include <kernel/mem.h>

#include <stdint.h>

void fb_draw_morse(volatile uint32_t *fb,
                   uint32_t pitch,
                   uint32_t x,
                   uint32_t y,
                   uint32_t color)
{
    const char *morse =
        "-. ---   ...- --. .-   ... ..- .--. .--. --- .-. -";

    while (*morse)
    {
        if (*morse == '.')
        {
            fb[y * pitch + x] = color;
            x += 2;              // 1 pixel + 1 pixel gap
        }
        else if (*morse == '-')
        {
            fb[y * pitch + x + 0] = color;
            fb[y * pitch + x + 1] = color;
            fb[y * pitch + x + 2] = color;
            x += 4;              // 3 pixels + 1 pixel gap
        }
        else if (*morse == ' ')
        {
            /* Detect word gap (three spaces) */
            if (morse[1] == ' ' && morse[2] == ' ')
            {
                x += 5;
                morse += 2;
            }
            else
            {
                /* Letter gap */
                x += 5;
            }
        }

        morse++;
    }
}

void vga_test(void)
{
    BOOT_INFO *boot =
        (BOOT_INFO*)KSP;


    volatile uint32_t *fb =
        (volatile uint32_t *)boot->framebuffer;

    fb_draw_morse(fb, boot->pitch, 0, 0, 0x00FF0000);

    // kprintf("FB=%x\n", (uint32_t)boot->framebuffer);
    // kprintf("SIZE=%x\n", (uint32_t)     boot->framebuffer_size);
    // kprintf("W=%u H=%u P=%u F=%u\n",
    //     boot->width,
    //     boot->height,
    //     boot->pitch,
    //     boot->format
    // );
}
