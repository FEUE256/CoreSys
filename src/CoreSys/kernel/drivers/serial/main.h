#pragma once

#include <init/kargs.h>
#include <stdint.h>
#include <drivers/task/main.h>       // Task management functions


typedef unsigned char u8;
typedef unsigned short u16;

// ==============================
// COM1 base port
// ==============================
#define COM1 0x3F8

// ==============================
// Port I/O
// ==============================
static inline void outb(u16 port, u8 val)
{
    __asm__ volatile ("outb %0, %1"
                      :
                      : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile ("inb %1, %0"
                      : "=a"(ret)
                      : "Nd"(port));
    return ret;
}

// ==============================
// Status helpers
// ==============================
static inline int serial_is_transmit_empty(void)
{
    return inb(COM1 + 5) & 0x20;
}


// ==============================
// Output
// ==============================
static inline void serial_write_char(char c)
{
    while (!serial_is_transmit_empty());
    outb(COM1, (u8)c);
}

static inline void serial_write(const char *s)
{
    while (*s)
        serial_write_char(*s++);
}

static inline void serial_write_u64(uint64_t value)
{
    char buf[32];
    int i = 31;

    buf[i] = '\0';

    if (value == 0)
    {
        serial_write_char('0');
        return;
    }

    while (value > 0 && i > 0)
    {
        buf[--i] = '0' + (value % 10);
        value /= 10;
    }

    serial_write(&buf[i]);
}

static inline void serial_write_ptr(const void *p)
{
    uint64_t v = (uint64_t)p;

    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';

    const char *hex = "0123456789ABCDEF";

    for (int i = 0; i < 16; i++)
    {
        buf[2 + i] = hex[(v >> (60 - i * 4)) & 0xF];
    }

    buf[18] = '\0';

    serial_write(buf);
}

static inline void kprint(const char *s)
{
    while (*s)
        serial_write_char(*s++);
}

static inline int serial_received(void)
{
    return inb(COM1 + 5) & 1;
}

char serial_read_char()
{
    while (serial_received() == 0)
        ;

    return inb(COM1);
}

char kread(void)
{
    while (serial_received() == 0)
        ;

    return inb(COM1);
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// ==============================
// Utilities
// ==============================
static inline void serial_clear(cs_task* self)
{
    (void)self; // Unused parameter
    serial_write("\x1B[2J\x1B[H");
}

static inline void kclear(void)
{
    serial_write("\x1B[2J\x1B[H");
}

void k_clear(cs_task* self) {
    (void)self; // Unused parameter
    serial_write("\x1B[2J\x1B[H");
}

// ==============================
// Init
// ==============================
static inline void initSerial(cs_task* self)
{
    (void)self; // Unused parameter
    cs_task clear_task = {
        .name = "Serial Clear Task",
        .source_header = "drivers/serial/main.h",
        .entry = serial_clear
    };
    task_run(&clear_task); // Clear serial output

    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80); // DLAB on

    // Baud rate 115200 (correct divisor = 1)
    outb(COM1 + 0, 0x01);
    outb(COM1 + 1, 0x00);

    outb(COM1 + 3, 0x03); // 8N1
    outb(COM1 + 2, 0xC7); // FIFO enable
    outb(COM1 + 4, 0x0B); // RTS/DSR + IRQ enable (safe default)
}

// ==============================
// Deinit
// ==============================
static inline void deinitSerial(cs_task* self)
{
    (void)self; // Unused parameter
    // Disable all interrupts
    outb(COM1 + 1, 0x00);

    // Disable FIFO (clear buffers)
    outb(COM1 + 2, 0x00);

    // Drop modem control lines (safe idle state)
    outb(COM1 + 4, 0x00);

    // Optional: reset line control register to default (8N1, DLAB off)
    outb(COM1 + 3, 0x03);

    // Optional: clear any remaining data
    cs_task clear_task = {
        .name = "Serial Clear Task",
        .source_header = "drivers/serial/main.h",
        .entry = serial_clear
    };
    task_run(&clear_task); // Clear serial output
}
