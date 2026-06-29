#pragma once

#include <stdint.h>
#include <stddef.h>

#include <drivers/serial/main.h>
#include <drivers/ret/main.h>
#include <mod/types.h>

void tty_putc(char c);
void tty_write(const char *s);
void tty_loop();
void serial_write_char(char c);
char serial_read_char();
void execute_command(const char *cmd, int debug);

// =========================
// HAL ABI Constants
// =========================

enum HAL_CALL {
    HAL_DEV_NULL          = 0,
    HAL_EXECUTE_COMMAND   = 1,
    HAL_TTY_LOOP          = 2,
    HAL_TTY_WRITE         = 3,
    HAL_TTY_PUTC          = 4,
    HAL_OUTB              = 5,
    HAL_INB               = 6,
    HAL_OUTW              = 7,
    HAL_INW               = 8,
    HAL_SERIAL_TX_EMPTY   = 9,
    HAL_SERIAL_RECEIVED   = 10,
    HAL_SERIAL_WRITE_CHAR = 11,
    HAL_A_CHAR_PRINT      = 12,
    HAL_SERIAL_WRITE      = 13,
    HAL_SERIAL_WRITE_U64  = 14,
    HAL_SERIAL_WRITE_PTR  = 15,
    HAL_KPRINT            = 16,
    HAL_KPRINT_CHAR       = 17,
    HAL_KPRINT_U64        = 18,
    HAL_KPRINT_U8         = 19,
    HAL_KPRINTF           = 20,
    HAL_SERIAL_READ_CHAR  = 21,
    HAL_KREAD             = 22,
    HAL_SERIAL_CLEAR      = 23,
    HAL_KCLEAR            = 24,
    HAL_K_CLEAR           = 25,
    HAL_KSTRLEN           = 26,
    HAL_KPRINT_STR        = 27,
    HAL_KPRINT_UINT       = 28,
    HAL_KPRINT_INT        = 29,
    HAL_INIT_SERIAL       = 30,
    HAL_DEINIT_SERIAL     = 31,
    HAL_LED_DEMO          = 32,
    HAL_SHUTDOWN          = 33,
    HAL_REBOOT            = 34
};

// =========================
// HAL ABI Frame
// =========================

typedef struct {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
} hal_frame_t;

uint64_t halcall(hal_frame_t* frame);

static inline uint64_t hal_do(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx) {
    hal_frame_t frame;
    frame.rax = rax;
    frame.rdi = rdi;
    frame.rsi = rsi;
    frame.rdx = rdx;
    return halcall(&frame);
}

// =========================
// HAL WRAPPERS (ALL 35)
// =========================
static inline int hal_dev_null(int code) {
    return hal_do(HAL_DEV_NULL, code, 0, 0);
}

static inline void hal_execute_command(const char* cmd) {
    hal_do(HAL_EXECUTE_COMMAND, (uintptr_t)cmd, 0, 0);
}

static inline void hal_tty_loop(void) {
    hal_do(HAL_TTY_LOOP, 0, 0, 0);
}

static inline void hal_tty_write(const char* s) {
    hal_do(HAL_TTY_WRITE, (uintptr_t)s, 0, 0);
}

static inline void hal_tty_putc(char c) {
    hal_do(HAL_TTY_PUTC, (uintptr_t)(uint8_t)c, 0, 0);
}

static inline uint8_t hal_inb(uint16_t port) {
    return (uint8_t)hal_do(HAL_INB, port, 0, 0);
}

static inline uint16_t hal_inw(uint16_t port) {
    return (uint16_t)hal_do(HAL_INW, port, 0, 0);
}

static inline void hal_outb(uint16_t port, uint8_t val) {
    hal_do(HAL_OUTB, port, val, 0);
}

static inline void hal_outw(uint16_t port, uint16_t val) {
    hal_do(HAL_OUTW, port, val, 0);
}

static inline uint8_t hal_serial_tx_empty(void) {
    return (uint8_t)hal_do(HAL_SERIAL_TX_EMPTY, 0, 0, 0);
}

static inline uint8_t hal_serial_received(void) {
    return (uint8_t)hal_do(HAL_SERIAL_RECEIVED, 0, 0, 0);
}

static inline void hal_serial_write_char(char c) {
    hal_do(HAL_SERIAL_WRITE_CHAR, (uintptr_t)(uint8_t)c, 0, 0);
}

static inline void hal_a_char_print(char c) {
    hal_do(HAL_A_CHAR_PRINT, (uintptr_t)(uint8_t)c, 0, 0);
}

static inline void hal_serial_write(const char* s) {
    hal_do(HAL_SERIAL_WRITE, (uintptr_t)s, 0, 0);
}

static inline void hal_serial_write_u64(uint64_t v) {
    hal_do(HAL_SERIAL_WRITE_U64, v, 0, 0);
}

static inline void hal_serial_write_ptr(const void* p) {
    hal_do(HAL_SERIAL_WRITE_PTR, (uintptr_t)p, 0, 0);
}

static inline void hal_kprint(const char* s) {
    hal_do(HAL_KPRINT, (uintptr_t)s, 0, 0);
}

static inline void hal_kprint_char(char c) {
    hal_do(HAL_KPRINT_CHAR, (uintptr_t)(uint8_t)c, 0, 0);
}

static inline void hal_kprint_u64(uint64_t v) {
    hal_do(HAL_KPRINT_U64, v, 0, 0);
}

static inline void hal_kprint_u8(uint8_t v) {
    hal_do(HAL_KPRINT_U8, v, 0, 0);
}

static inline void hal_kprintf(const char* s) {
    hal_do(HAL_KPRINTF, (uintptr_t)s, 0, 0);
}

static inline char hal_serial_read_char(void) {
    return (char)hal_do(HAL_SERIAL_READ_CHAR, 0, 0, 0);
}

static inline char hal_kread(void) {
    return (char)hal_do(HAL_KREAD, 0, 0, 0);
}

static inline void hal_serial_clear(void) {
    hal_do(HAL_SERIAL_CLEAR, 0, 0, 0);
}

static inline void hal_kclear(void) {
    hal_do(HAL_KCLEAR, 0, 0, 0);
}

static inline void hal_k_clear(void) {
    hal_do(HAL_K_CLEAR, 0, 0, 0);
}

static inline size_t hal_kstrlen(const char* s) {
    return (size_t)hal_do(HAL_KSTRLEN, (uintptr_t)s, 0, 0);
}

static inline void hal_kprint_str(const char* s) {
    hal_do(HAL_KPRINT_STR, (uintptr_t)s, 0, 0);
}

static inline void hal_kprint_uint(uint64_t v, unsigned base) {
    hal_do(HAL_KPRINT_UINT, v, base, 0);
}

static inline void hal_kprint_int(int64_t v) {
    hal_do(HAL_KPRINT_INT, (uint64_t)v, 0, 0);
}

static inline void hal_init_serial(void) {
    hal_do(HAL_INIT_SERIAL, 0, 0, 0);
}

static inline void hal_deinit_serial(void) {
    hal_do(HAL_DEINIT_SERIAL, 0, 0, 0);
}

static inline void hal_led_demo(void) {
    hal_do(HAL_LED_DEMO, 0, 0, 0);
}

static inline void hal_shutdown(void) {
    hal_do(HAL_SHUTDOWN, 0, 0, 0);
}

static inline void hal_reboot(void) {
    hal_do(HAL_REBOOT, 0, 0, 0);
}
