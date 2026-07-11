#pragma once

#include <stdint.h>
#include <drivers/serial/main.h>
#include <drivers/tty/cmd.h>
#include <drivers/task/main.h>      // Task management functions
#include <drivers/cfs/main.h>

void tty_putc(char c);
void tty_write(const char *s);
void tty_loop();

// You already have this somewhere
extern void serial_write_char(char c);
extern char serial_read_char();
extern void execute_command(const char *cmd, int debug);

static char buffer[128];
static int index = 0;

void tty_putc(char c)
{
    serial_write_char(c);
}

void tty_write(const char *s)
{
    while (*s)
        tty_putc(*s++);
}

void tty_loop()
{
    uint64_t size;
    char *data = (char *)cfs_read(debug_cfg_file, &size);

    uint64_t debug = 0;

    if (data)
    {
        data[size] = '\0';
        debug = kstrtoull(data, NULL, 10);
    }

    if (debug != 2) { tty_write("[LOG] CoreSys Terminal Ready (type help for Help)\r\n/sys/system/> "); }
    if (debug == 2) { tty_write("/sys/system/> "); }

    while (1)
    {
        char c = serial_read_char(); // Read a character from the serial port

        if (c == '\r' || c == '\n')
        {
            buffer[index] = 0;
            tty_write("\r\n");

            execute_command(buffer, debug);

            index = 0;
            tty_write("/sys/system/> ");
        }
        else if (index < (int)(sizeof(buffer) - 1))
        {
            buffer[index++] = c;
            tty_putc(c);
        }
    }
}
