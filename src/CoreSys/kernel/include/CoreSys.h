#pragma once

/* =========================
   SAFE GCC / MSVC COMPAT x86 CORE
   ========================= */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdnoreturn.h>

/* x86 baseline intrinsics */
#include <x86intrin.h>
#include <x86gprintrin.h>
#include <immintrin.h>

#include <emmintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <wmmintrin.h>
#include <immintrin.h>

#include <mm_malloc.h>
#include <limits.h>
#include <float.h>
#include <syslimits.h>

/* optional safe CPU features */
#include <cpuid.h>
#include <lzcntintrin.h>
#include <popcntintrin.h>
#include <rdseedintrin.h>
#include <rtmintrin.h>
#include <waitpkgintrin.h>
#include <fxsrintrin.h>

/* debug / toolchain */
#include <gcov.h>
#include <unwind.h>

/* OpenMP (only if you really use it) */
#include <omp.h>

/* =========================
   YOUR CORE SYSTEM HEADERS
   ========================= */

#include <globe.h>

#include <drivers/ret/main.h>

#include <drivers/ACPI/main.h>
#include <drivers/led/main.h>
#include <drivers/serial/main.h>
#include <drivers/time/main.h>

#include <drivers/tty/main.h>
#include <drivers/tty/cmd.h>

#include <drivers/halcalls/main.h>
#include <drivers/hal/main.h>

#include <drivers/init/main.h>
#include <drivers/task/main.h>
#include <drivers/halt/main.h>
#include <drivers/sf/main.h>

#include <drivers/syscalls/main.h>
#include <drivers/sys/main.h>

#include <drivers/cfs/main.h>
#include <drivers/log/main.h>
#include <drivers/page/main.h>
#include <drivers/reg/main.h>

void execute_command(const char *cmd, int debug);
void tty_putc(char c);
void tty_write(const char *s);
void tty_loop(int debug);
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
int serial_received(void);
void serial_write_char(char c);
void a_char_print(char c);
void serial_write(const char *s);
void serial_write_u64(uint64_t value);
void serial_write_ptr(const void *p);
void kprint(const char *s);
void kprint_char(char c);
void kprint_u64(uint64_t v);
void kprint_u8(uint8_t v);
int kprintf(const char *fmt, ...);
char serial_read_char(void);
char kread(void);
void kclear(void);
void k_clear(cs_task* self);
int kstrlen(const char *s);
void kprint_str(const char *s);
void kprint_uint(unsigned long long v, unsigned base);
void kprint_int(long long v);
void initSerial(cs_task* self);
void deinitSerial(cs_task* self);
void shutdown(void);
void reboot(void);

// System functions
char sys_read(void);
void sys_write(const char* s);
void sys_shutdown(void);
void sys_reboot(void);
void sys_init(void);
void sys_deinit(void);
void sys_clear(void);
void sys_reinit(void);
void sys_halt(void);
void sys_sf(const char* s);

typedef struct CS_HAL {
    int (*null)(int);

    void (*execute_command)(const char*);
    void (*tty_loop)(void);
    void (*tty_write)(const char*);
    void (*tty_putc)(char);

    void (*outb)(uint16_t, uint8_t);
    uint8_t (*inb)(uint16_t);
    void (*outw)(uint16_t, uint16_t);
    uint16_t (*inw)(uint16_t);

    uint8_t (*serial_received)(void);
    void (*serial_write_char)(char);
    void (*a_char_print)(char);

    void (*serial_write)(const char*);
    void (*serial_write_u64)(uint64_t);
    void (*serial_write_ptr)(const void*);

    void (*kprint)(const char*);
    void (*kprint_char)(char);
    void (*kprint_u64)(uint64_t);
    void (*kprint_u8)(uint8_t);
    void (*kprintf)(const char*);

    char (*serial_read_char)(void);
    char (*kread)(void);

    void (*kclear)(void);
    void (*k_clear)(void);

    size_t (*kstrlen)(const char*);

    void (*kprint_str)(const char*);
    void (*kprint_uint)(unsigned long long, unsigned);
    void (*kprint_int)(long long);

    void (*init_serial)(void);
    void (*deinit_serial)(void);

    void (*shutdown)(void);
    void (*reboot)(void);
} CS_HAL;

typedef struct CS_SYS {
    int  (*null)(void);
    char (*read)(void);
    void (*write)(const char*);

    void (*shutdown)(void);
    void (*reboot)(void);

    void (*init)(void);
    void (*deinit)(void);
    void (*clear)(void);
    void (*reinit)(void);
    void (*halt)(void);

    void (*sf)(const char*);
} CS_SYS;

typedef struct CS_CORE {
    CS_HAL hal;
    CS_SYS sys;
} CS_CORE;

void tsk_init(void) {
    tsk_ready = 1;
}

void tsk_deinit(void) {
    tsk_ready = 0;
}

void cs_init(CS_CORE *core)
{
    tsk_init();

    core->hal.null              = hal_dev_null;

    core->hal.execute_command   = hal_execute_command;
    core->hal.tty_loop          = hal_tty_loop;
    core->hal.tty_write         = hal_tty_write;
    core->hal.tty_putc          = hal_tty_putc;

    core->hal.outb              = hal_outb;
    core->hal.inb               = hal_inb;
    core->hal.outw              = hal_outw;
    core->hal.inw               = hal_inw;

    core->hal.serial_received   = hal_serial_received;
    core->hal.serial_write_char = hal_serial_write_char;
    core->hal.a_char_print      = hal_a_char_print;

    core->hal.serial_write      = hal_serial_write;
    core->hal.serial_write_u64  = hal_serial_write_u64;
    core->hal.serial_write_ptr  = hal_serial_write_ptr;

    core->hal.kprint            = hal_kprint;
    core->hal.kprint_char       = hal_kprint_char;
    core->hal.kprint_u64        = hal_kprint_u64;
    core->hal.kprint_u8         = hal_kprint_u8;
    core->hal.kprintf           = hal_kprintf;

    core->hal.serial_read_char  = hal_serial_read_char;
    core->hal.kread             = hal_kread;

    core->hal.kclear            = hal_kclear;
    core->hal.k_clear           = hal_k_clear;

    core->hal.kstrlen           = hal_kstrlen;

    core->hal.kprint_str        = hal_kprint_str;
    core->hal.kprint_uint       = hal_kprint_uint;
    core->hal.kprint_int        = hal_kprint_int;

    core->hal.init_serial       = hal_init_serial;
    core->hal.deinit_serial     = hal_deinit_serial;

    core->hal.shutdown          = hal_shutdown;
    core->hal.reboot            = hal_reboot;

    core->sys.null      = sys_dev_null;
    core->sys.read      = sys_read;
    core->sys.write     = sys_write;

    core->sys.shutdown = sys_shutdown;
    core->sys.reboot   = sys_reboot;

    core->sys.init      = sys_init;
    core->sys.deinit    = sys_deinit;
    core->sys.clear     = sys_clear;
    core->sys.reinit    = sys_reinit;
    core->sys.halt      = sys_halt;
    core->sys.sf        = sys_sf;
}

void cs_deinit(CS_CORE *core)
{
    tsk_deinit();

    core->hal.execute_command   = NULL;
    core->hal.tty_loop          = NULL;
    core->hal.tty_write         = NULL;
    core->hal.tty_putc          = NULL;

    core->hal.outb              = NULL;
    core->hal.inb               = NULL;
    core->hal.outw              = NULL;
    core->hal.inw               = NULL;

    core->hal.serial_received   = NULL;
    core->hal.serial_write_char = NULL;
    core->hal.a_char_print      = NULL;

    core->hal.serial_write      = NULL;
    core->hal.serial_write_u64  = NULL;
    core->hal.serial_write_ptr  = NULL;

    core->hal.kprint            = NULL;
    core->hal.kprint_char       = NULL;
    core->hal.kprint_u64        = NULL;
    core->hal.kprint_u8         = NULL;
    core->hal.kprintf           = NULL;

    core->hal.serial_read_char  = NULL;
    core->hal.kread             = NULL;

    core->hal.kclear            = NULL;
    core->hal.k_clear           = NULL;

    core->hal.kstrlen           = NULL;
    core->hal.kprint_str        = NULL;
    core->hal.kprint_uint       = NULL;
    core->hal.kprint_int        = NULL;

    core->hal.init_serial       = NULL;
    core->hal.deinit_serial     = NULL;

    core->hal.shutdown          = NULL;
    core->hal.reboot            = NULL;

    core->sys.read             = NULL;
    core->sys.write            = NULL;

    core->sys.shutdown         = NULL;
    core->sys.reboot           = NULL;

    core->sys.init              = NULL;
    core->sys.deinit            = NULL;
    core->sys.clear             = NULL;
    core->sys.reinit            = NULL;
    core->sys.halt              = NULL;
    core->sys.sf                = NULL;
}
