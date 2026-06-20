#pragma once

#include <stdint.h>
#include <drivers/task/main.h>       // Task management functions
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <drivers/halt/main.h>       // For halt() function

typedef unsigned char u8;
typedef unsigned short u16;

// ==============================
// COM base port
// ==============================
#define COM1 0x3F8 
#define a_port 0x2E8

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

static inline void a_char_print(char c)
{
    while (!serial_is_transmit_empty());
    outb(a_port, (u8)c);
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

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile(
        "outl %0,%1"
        :
        : "a"(value),
          "Nd"(port)
    );
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t r;

    __asm__ volatile(
        "inl %1,%0"
        : "=a"(r)
        : "Nd"(port)
    );

    return r;
}

static inline void kprint(const char *s)
{
    while (*s)
        serial_write_char(*s++);
}

static inline void kprint_char(char c)
{
    while (!serial_is_transmit_empty());
    outb(COM1, (u8)c);
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

static int is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static int str_to_int(const char *s)
{
    int result = 0;
    int sign = 1;

    if (*s == '-')
    {
        sign = -1;
        s++;
    }

    while (*s)
    {
        if (!is_digit(*s))
            break;

        result = result * 10 + (*s - '0');
        s++;
    }

    return result * sign;
}

/*
    kscanf - minimal blocking console input from serial
    supports:
        %d  (int)
        %u  (unsigned int)
        %s  (string, whitespace terminated)
*/
int kscanf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char buffer[128];
    int buf_i = 0;

    for (int i = 0; fmt[i]; i++)
    {
        if (fmt[i] != '%')
            continue;

        char spec = fmt[++i];

        /* clear buffer */
        buf_i = 0;

        /* read until newline / space */
        while (1)
        {
            char c = serial_read_char();

            /* echo */
            serial_write_char(c);

            if (c == '\r' || c == '\n' || c == ' ')
                break;

            if (buf_i < (int)sizeof(buffer) - 1)
                buffer[buf_i++] = c;
        }

        buffer[buf_i] = '\0';

        switch (spec)
        {
            case 'd':
            {
                int *out = va_arg(ap, int*);
                *out = str_to_int(buffer);
                break;
            }

            case 'u':
            {
                unsigned int *out = va_arg(ap, unsigned int*);
                *out = (unsigned int)str_to_int(buffer);
                break;
            }

            case 's':
            {
                char *out = va_arg(ap, char*);
                for (int j = 0; j <= buf_i; j++)
                    out[j] = buffer[j];
                break;
            }

            default:
                break;
        }
    }

    va_end(ap);
    return 0;
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

void kprint_u64(uint64_t v)
{
    char buf[32];
    char *p = buf + 31;
    *p = 0;

    do {
        *--p = '0' + (v % 10);
        v /= 10;
    } while (v);

    kprint(p);
}

static int kstrlen(const char *s)
{
    int i = 0;
    while (s && s[i]) i++;
    return i;
}

static void kprint_str(const char *s)
{
    if (!s) { kprint("(null)"); return; }
    kprint(s);
}

static void kprint_uint(unsigned long long v, unsigned base)
{
    char buf[32];
    const char *digits = "0123456789abcdef";
    int i = 0;

    if (v == 0)
    {
        kprint_char('0');
        return;
    }

    while (v > 0)
    {
        buf[i++] = digits[v % base];
        v /= base;
    }

    while (i--)
        kprint_char(buf[i]);
}

static void kprint_int(long long v)
{
    if (v < 0)
    {
        kprint_char('-');
        kprint_uint((unsigned long long)(-v), 10);
    }
    else
    {
        kprint_uint((unsigned long long)v, 10);
    }
}

void kprint_u8(uint8_t v)
{
    char buf[4];

    buf[0] = '0' + (v / 10);
    buf[1] = '0' + (v % 10);
    buf[2] = '\n';
    buf[3] = 0;

    kprint(buf);
}

void kitoa_base(unsigned long v, int base, char *buf)
{
    const char *digits = "0123456789ABCDEF";

    char tmp[32];
    int i = 0;

    if (v == 0)
    {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }

    while (v > 0)
    {
        tmp[i++] = digits[v % base];
        v /= base;
    }

    int j = 0;
    while (i--)
        buf[j++] = tmp[i];

    buf[j] = 0;
}

int kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int count = 0;

    for (int i = 0; fmt && fmt[i]; i++)
    {
        if (fmt[i] != '%')
        {
            kprint_char(fmt[i]);
            count++;
            continue;
        }

        i++;

        // -----------------------------
        // FLAGS
        // -----------------------------
        int zero_pad = 0;

        if (fmt[i] == '0')
        {
            zero_pad = 1;
            i++;
        }

        // -----------------------------
        // WIDTH PARSING
        // -----------------------------
        int width = 0;

        while (fmt[i] >= '0' && fmt[i] <= '9')
        {
            width = width * 10 + (fmt[i] - '0');
            i++;
        }

        char c = fmt[i];

        switch (c)
        {
            case 's':
            {
                const char *s = va_arg(ap, const char *);
                if (!s) s = "(null)";

                kprint_str(s);
                count += kstrlen(s);
                break;
            }

            case 'c':
            {
                char ch = (char)va_arg(ap, int);
                kprint_char(ch);
                count++;
                break;
            }

            case 'd':
            case 'i':
            {
                int v = va_arg(ap, int);
                kprint_int(v);
                break;
            }

            case 'u':
            {
                unsigned int v = va_arg(ap, unsigned int);
                kprint_uint(v, 10);
                break;
            }

            case 'x':
            {
                unsigned int v = va_arg(ap, unsigned int);

                char buf[32];
                kitoa_base(v, 16, buf);

                int len = kstrlen(buf);

                // padding
                for (int p = len; p < width; p++)
                    kprint_char(zero_pad ? '0' : ' ');

                kprint_str(buf);
                count += len;
                break;
            }

            case 'p':
            {
                void *p = va_arg(ap, void *);
                kprint("0x");

                char buf[32];
                kitoa_base((uintptr_t)p, 16, buf);

                kprint_str(buf);
                count += 2 + kstrlen(buf);
                break;
            }

            case '%':
            {
                kprint_char('%');
                count++;
                break;
            }

            default:
            {
                kprint_char('%');
                kprint_char(c);
                count += 2;
                break;
            }
        }
    }

    va_end(ap);
    return count;
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

    outb(a_port + 1, 0x00); // disable interrupts
    outb(a_port + 3, 0x80); // DLAB on

    // Baud rate 115200 (correct divisor = 1)
    outb(a_port + 0, 0x01);
    outb(a_port + 1, 0x00);

    outb(a_port + 3, 0x03); // 8N1
    outb(a_port + 2, 0xC7); // FIFO enable
    outb(a_port + 4, 0x0B); // RTS/DSR + IRQ enable (safe default)
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


static void out_char(char **buf, size_t *n, char c)
{
    if (*n > 1) {
        **buf = c;
        (*buf)++;
        (*n)--;
    }
}

static void out_str(char **buf, size_t *n, const char *s)
{
    while (*s) {
        out_char(buf, n, *s++);
    }
}

static void out_uint(char **buf, size_t *n, uint64_t v, int base, bool upper)
{
    char tmp[65];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;

    if (v == 0) {
        out_char(buf, n, '0');
        return;
    }

    while (v && i < (int)sizeof(tmp)) {
        tmp[i++] = digits[v % base];
        v /= base;
    }

    while (i--) {
        out_char(buf, n, tmp[i]);
    }
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    char *start = buf;
    size_t n = size;

    if (n == 0) {
        return 0;
    }

    while (*fmt) {
        if (*fmt != '%') {
            out_char(&buf, &n, *fmt++);
            continue;
        }

        fmt++; // skip %

        switch (*fmt) {
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                out_str(&buf, &n, s);
                break;
            }

            case 'c': {
                char c = (char)va_arg(args, int);
                out_char(&buf, &n, c);
                break;
            }

            case 'd':
            case 'i': {
                int v = va_arg(args, int);
                if (v < 0) {
                    out_char(&buf, &n, '-');
                    out_uint(&buf, &n, (uint64_t)(-v), 10, false);
                } else {
                    out_uint(&buf, &n, (uint64_t)v, 10, false);
                }
                break;
            }

            case 'u': {
                unsigned int v = va_arg(args, unsigned int);
                out_uint(&buf, &n, v, 10, false);
                break;
            }

            case 'x': {
                unsigned int v = va_arg(args, unsigned int);
                out_uint(&buf, &n, v, 16, false);
                break;
            }

            case 'X': {
                unsigned int v = va_arg(args, unsigned int);
                out_uint(&buf, &n, v, 16, true);
                break;
            }

            case 'p': {
                uintptr_t v = (uintptr_t)va_arg(args, void *);
                out_str(&buf, &n, "0x");
                out_uint(&buf, &n, v, 16, false);
                break;
            }

            case '%': {
                out_char(&buf, &n, '%');
                break;
            }

            default:
                out_char(&buf, &n, '%');
                out_char(&buf, &n, *fmt);
                break;
        }

        fmt++;
    }

    *buf = '\0';

    return (int)(buf - start);
}


static void reverse(char *s, int len)
{
    int i = 0;
    int j = len - 1;

    while (i < j)
    {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}

static int itoa_simple(int value, char *buf)
{
    int i = 0;
    int is_negative = 0;

    if (value == 0)
    {
        buf[i++] = '0';
        buf[i] = '\0';
        return 1;
    }

    if (value < 0)
    {
        is_negative = 1;
        value = -value;
    }

    while (value > 0)
    {
        buf[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative)
        buf[i++] = '-';

    buf[i] = '\0';

    reverse(buf, i);

    return i;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    size_t pos = 0;

    for (size_t i = 0; fmt[i] != '\0' && pos < size - 1; i++)
    {
        if (fmt[i] == '%' && fmt[i + 1] == 'd')
        {
            int val = va_arg(args, int);

            char num[32];
            int len = itoa_simple(val, num);

            for (int j = 0; j < len && pos < size - 1; j++)
                buf[pos++] = num[j];

            i++; // skip 'd'
        }
        else if (fmt[i] == '%' && fmt[i + 1] == 's')
        {
            const char *str = va_arg(args, const char*);

            for (int j = 0; str[j] != '\0' && pos < size - 1; j++)
                buf[pos++] = str[j];

            i++; // skip 's'
        }
        else
        {
            buf[pos++] = fmt[i];
        }
    }

    buf[pos] = '\0';

    va_end(args);

    return pos;
}
