// --------------------------------
// FÈUE File CoreSys Kernel
// Contains code of the project
// CyberBoot by FÈUE
// --------------------------------

#include <init/kargs.h>        // Kernel arguments structure
#include <stdint.h>             // Standard integer types

typedef unsigned char u8;
typedef unsigned short u16;

// COM1 base port
#define COM1 0x3F8

// I/O port write
static inline void outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// I/O port read
static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Wait for serial port to be ready
int serial_is_transmit_empty() {
    return inb(COM1 + 5) & 0x20;
}

// Write a character
void serial_write_char(char c) {
    while (!serial_is_transmit_empty());
    outb(COM1, c);
}

// Write a string
void serial_write(const char *s) {
    for (; *s; s++) serial_write_char(*s);
}

void serial_clear() {
    // ESC[2J clears screen, ESC[H moves cursor to top-left
    serial_write("\x1B[2J\x1B[H");
}

int kmain(kargs *Args) {
    // Initialize COM1: 115200 baud, 8N1
    outb(COM1 + 1, 0x00);    // Disable interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB
    outb(COM1 + 0, 0x01);    // Divisor low byte (115200)
    outb(COM1 + 1, 0x00);    // Divisor high byte
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // FIFO enabled, clear
    outb(COM1 + 4, 0x0B);    // IRQs, RTS/DSR

    serial_clear();
    serial_write("Hello from CoreSys kernel via serial at 115200 baud rate!\n");
    while (1) {
        // Infinite loop to keep the kernel running
    }
    return Args->DAUDA;
}
