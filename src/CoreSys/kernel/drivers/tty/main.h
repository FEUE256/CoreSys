#pragma once

#include <stdint.h>
#include <drivers/serial/main.h>
#include <drivers/tty/cmd.h>
#include <drivers/task/main.h>      // Task management functions
#include <drivers/cfs/main.h>
#include <drivers/tty/state.h>

void tty_putc(char c);
void tty_write(const char *s);
void tty_loop();

// You already have this somewhere
extern void serial_write_char(char c);
extern char serial_read_char();
extern void execute_command(const char *cmd, int debug);

void tty_putc(char c)
{
    serial_write_char(c);
}

void tty_write(const char *s)
{
    while (*s)
        tty_putc(*s++);
}

void tty_run(cs_task *self) {
    (void)self;
    tty_loop();
}

void tty_loop(void)
{
    uint64_t size;
    char *data = (char *)cfs_read(debug_cfg_file, &size);
    
    uint64_t debug = 0;
    if (data) { data[size] = '\0'; debug = kstrtoull(data, NULL, 10); }

    // init all terminals
    for (int i = 0; i < TTY_MAX_TERMS; i++) {
        tty_terms[i].index  = 0;
        tty_terms[i].active = 0;
        tty_terms[i].cancel = 0;
        tty_terms[i].buffer[0] = '\0';
        strcpy(tty_terms[i].cwd, "/");   // each TTY starts at root
    }
    tty_terms[0].active = 1;

    if (debug != 2) { tty_write("[LOG] CoreSys Terminal Ready (type help for Help, Ctrl+T for next terminal, '_' to cancel)\r\n"); }
    kprintf("[TTY0] %s> ", tty_terms[0].cwd);

    while (1)
    {
        char c = serial_read_char();

        // Ctrl+T = switch terminal
        if (c == '\x14') {
            tty_current = (tty_current + 1) % TTY_MAX_TERMS;
            tty_terms[tty_current].active = 1;

            tty_write("\r\n");
            kprintf("[TTY%d] %s> ", tty_current, tty_terms[tty_current].cwd);

            tty_state_t *switched = &tty_terms[tty_current];
            if (switched->index > 0) {
                tty_write(switched->buffer);
            }
            continue;
        }
        // cancel
        else if (c == '_') {
            tty_terms[tty_current].cancel = 1;
            tty_terms[tty_current].index = 0;
            tty_terms[tty_current].buffer[0] = '\0';
            tty_write("\r\n");
            kprintf("[TTY%d] %s> ", tty_current, tty_terms[tty_current].cwd);
            continue;
        }
        // enter
        else if (c == '\r' || c == '\n') {
            tty_state_t *term = &tty_terms[tty_current];
            term->buffer[term->index] = '\0';
            tty_write("\r\n");

            execute_command(term->buffer, (int)debug);

            term->index = 0;
            term->buffer[0] = '\0';
            tty_write("\r\n");
            kprintf("[TTY%d] %s> ", tty_current, tty_terms[tty_current].cwd);
        }
        // backspace
        else if (c == '\x7f' || c == '\b') {
            tty_state_t *term = &tty_terms[tty_current];
            if (term->index > 0) {
                term->index--;
                term->buffer[term->index] = '\0';
                tty_write("\b \b");
            }
        }
        // normal character
        else {
            tty_state_t *term = &tty_terms[tty_current];
            if (term->index < TTY_BUF_SIZE - 1) {
                term->buffer[term->index++] = c;
                term->buffer[term->index]   = '\0';
                tty_putc(c);
            }
        }
    }
}
