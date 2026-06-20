#ifndef IO_PORT_H
#define IO_PORT_H

#include <stdint.h>

/* Byte I/O Operations */
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* Word I/O Operations */
static inline void outw(uint16_t port, uint16_t value) {
    asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* Double Word I/O Operations */
static inline void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t result;
    asm volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* String I/O Operations */
static inline void outsb(uint16_t port, const uint8_t *addr, uint32_t count) {
    asm volatile("rep outsb" : "+S"(addr), "+c"(count) : "d"(port));
}

static inline void insb(uint16_t port, uint8_t *addr, uint32_t count) {
    asm volatile("rep insb" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

static inline void outsw(uint16_t port, const uint16_t *addr, uint32_t count) {
    asm volatile("rep outsw" : "+S"(addr), "+c"(count) : "d"(port));
}

static inline void insw(uint16_t port, uint16_t *addr, uint32_t count) {
    asm volatile("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

static inline void outsl(uint16_t port, const uint32_t *addr, uint32_t count) {
    asm volatile("rep outsl" : "+S"(addr), "+c"(count) : "d"(port));
}

static inline void insl(uint16_t port, uint32_t *addr, uint32_t count) {
    asm volatile("rep insl" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

/* Delayed I/O Operations */
static inline void outb_delay(uint16_t port, uint8_t value) {
    outb(port, value);
    asm volatile("nop; nop; nop; nop;");
}

static inline void outw_delay(uint16_t port, uint16_t value) {
    outw(port, value);
    asm volatile("nop; nop; nop; nop;");
}

static inline uint8_t inb_delay(uint16_t port) {
    uint8_t result = inb(port);
    asm volatile("nop; nop; nop; nop;");
    return result;
}

static inline uint16_t inw_delay(uint16_t port) {
    uint16_t result = inw(port);
    asm volatile("nop; nop; nop; nop;");
    return result;
}

#endif /* IO_PORT_H */
