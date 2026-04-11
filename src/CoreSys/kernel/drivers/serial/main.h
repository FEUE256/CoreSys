#pragma once

#include <init/kargs.h>
#include <stdint.h>

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

static inline void kprint(const char *s)
{
    while (*s)
        serial_write_char(*s++);
}

static inline int serial_received(void)
{
    return inb(COM1 + 5) & 1;
}

char serial_read_char(void)
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
static inline void serial_clear(void)
{
    serial_write("\x1B[2J\x1B[H");
}

static inline void kclear(void)
{
    serial_write("\x1B[2J\x1B[H");
}

// ==============================
// Init
// ==============================
static inline void initSerial(void)
{
    serial_clear();

    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80); // DLAB on

    // Baud rate 115200 (correct divisor = 1)
    outb(COM1 + 0, 0x01);
    outb(COM1 + 1, 0x00);

    outb(COM1 + 3, 0x03); // 8N1
    outb(COM1 + 2, 0xC7); // FIFO enable
    outb(COM1 + 4, 0x0B); // RTS/DSR + IRQ enable (safe default)
}
