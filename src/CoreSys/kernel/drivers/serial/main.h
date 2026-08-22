#pragma once

#include <stdint.h>
#include <drivers/task/main.h>       // Task management functions
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <drivers/halt/main.h>       // For halt() function

typedef unsigned char u8;
typedef unsigned short u16;

void print_tid(tid_t tid);

// ==============================
// COM base port
// ==============================
#define COM1 0x3F8 
#define a_port 0x2E8

static inline uint64_t rdtsc()
{
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void delay_rough()
{
    for (volatile uint32_t i = 0; i < 50000; i++)
    {
        __asm__ volatile("nop");
    }
}

// ==============================
// Port I/O
// ==============================
inline void outb(u16 port, u8 val)
{
    __asm__ volatile ("outb %0, %1"
                      :
                      : "a"(val), "Nd"(port));
}

inline u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile ("inb %1, %0"
                      : "=a"(ret)
                      : "Nd"(port));
    return ret;
}

static inline void outsw(uint16_t port, const void *addr, uint32_t count)
{
    __asm__ volatile (
        "rep outsw"
        : "+S"(addr), "+c"(count)
        : "d"(port)
    );
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

void print_int_pad(uint64_t value, int width)
{
    uint64_t div = 1;

    for (int i = 1; i < width; i++)
        div *= 10;

    while (div)
    {
        serial_write_char('0' + (value / div));
        value %= div;
        div /= 10;
    }
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

static void out_char(char **buf, size_t *n, char c)
{
    if (*n > 1) {
        **buf = c;
        (*buf)++;
        (*n)--;
    }
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
            
            case 't':
            {
                char *tid = va_arg(ap, char *);

                print_tid(tid);

                count += 16;
                break;
            }

            case 'a':
            {
                int64_t value = va_arg(ap, int64_t);

                if (value == INT64_MIN)
                {
                    kprint("-9223372036854775.808");
                    break;
                }

                if (value < 0)
                {
                    kprint_char('-'); 
                    value = -value;
                }

                kprint_int(value / 1000);
                kprint_char('.');
                print_int_pad(value % 1000, 3);

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

            case 'q':
            {
                const uint8_t *digest = va_arg(ap, const uint8_t *);
                const char hex[] = "0123456789abcdef";

                for (unsigned int i = 0; i < 32; i++)
                {
                    unsigned int v = digest[i];

                    kprint_char(hex[(v >> 4) & 0xF]);
                    kprint_char(hex[v & 0xF]);

                    count += 2;
                }

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

#include <stdint.h>

void int_to_ascii(int n, char *out)
{
    int i = 0;
    int sign = 0;

    if (n == 0) {
        out[i++] = '0';
        out[i] = '\0';
        return;
    }

    if (n < 0) {
        sign = 1;
        n = -n;
    }

    while (n > 0) {
        out[i++] = (n % 10) + '0';
        n /= 10;
    }

    if (sign) {
        out[i++] = '-';
    }

    out[i] = '\0';

    // reverse string
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char tmp = out[j];
        out[j] = out[k];
        out[k] = tmp;
    }
}

char* strcpy(char* dest, const char* src)
{
    char* start = dest;

    while (*src)
    {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';

    return start;
}

static inline void insw(uint16_t port, void *addr, uint32_t count) {
    __asm__ volatile ("rep insw"
        : "+D"(addr), "+c"(count)
        : "d"(port)
        : "memory");
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
        .entry_name = "serial_clear",
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
        .entry_name = "serial_clear",
        .entry = serial_clear
    };
    task_run(&clear_task); // Clear serial output
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
        else if (fmt[i] == '%' && fmt[i + 1] == 'p')
        {
            void *ptr = va_arg(args, void *);

            uintptr_t value = (uintptr_t)ptr;

            const char hex[] = "0123456789ABCDEF";

            if (pos < size - 1)
                buf[pos++] = '0';

            if (pos < size - 1)
                buf[pos++] = 'x';

            for (int shift = (sizeof(uintptr_t) * 8) - 4; shift >= 0; shift -= 4)
            {
                if (pos < size - 1)
                    buf[pos++] = hex[(value >> shift) & 0xF];
            }

            i++; // skip 'p'
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

#include <stdint.h>
#include <stddef.h>

/* ---------------- memcmp ---------------- */
int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a;
    const unsigned char *y = b;

    for (size_t i = 0; i < n; i++)
    {
        if (x[i] != y[i])
            return x[i] - y[i];
    }
    return 0;
}

/* ---------------- strchr ---------------- */
char *strchr(const char *s, int c)
{
    while (*s)
    {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }
    return NULL;
}

/* ---------------- strspn ---------------- */
size_t strspn(const char *s, const char *accept)
{
    const char *p = s;

    while (*p)
    {
        const char *a = accept;
        int found = 0;

        while (*a)
        {
            if (*p == *a)
            {
                found = 1;
                break;
            }
            a++;
        }

        if (!found)
            break;

        p++;
    }

    return p - s;
}

/* ---------------- strcspn ---------------- */
size_t strcspn(const char *s, const char *reject)
{
    const char *p = s;

    while (*p)
    {
        const char *r = reject;

        while (*r)
        {
            if (*p == *r)
                return p - s;

            r++;
        }

        p++;
    }

    return p - s;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n > 0) {
        unsigned char c1 = (unsigned char)*s1;
        unsigned char c2 = (unsigned char)*s2;

        if (c1 != c2)
            return c1 - c2;

        if (c1 == '\0')
            return 0;

        s1++;
        s2++;
        n--;
    }

    return 0;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i = 0;

    while (i < n && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }

    while (i < n) {
        dest[i] = '\0';
        i++;
    }

    return dest;
}

uint64_t kstrtoull(const char *str, const char **endptr, int base)
{
    uint64_t result = 0;

    if (!str) {
        if (endptr) *endptr = NULL;
        return 0;
    }

    // skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' ||
           *str == '\r' || *str == '\f' || *str == '\v') {
        str++;
    }

    // optional sign (strtoull allows '-', but result is unsigned)
    if (*str == '+') {
        str++;
    } else if (*str == '-') {
        str++;
    }

    // base auto-detect
    if (base == 0) {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            base = 16;
        } else if (str[0] == '0') {
            base = 8;
        } else {
            base = 10;
        }
    }

    // skip 0x prefix
    if (base == 16 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }

    while (*str) {
        char c = *str;
        uint8_t digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            break;
        }

        if (digit >= base) {
            break;
        }

        result = result * (uint64_t)base + digit;
        str++;
    }

    if (endptr) {
        *endptr = str;
    }

    return result;
}

void bc_to_buf(uint64_t bc, char *buf, int buf_size)
{
    if (!buf || buf_size <= 1)
        return;

    int i = 0;

    if (bc == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (bc > 0 && i < buf_size - 1)
    {
        int digit = bc % 10;
        buf[i++] = '0' + digit;
        bc /= 10;
    }

    /* reverse */
    for (int j = 0; j < i / 2; j++)
    {
        char tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = tmp;
    }

    buf[i] = '\0';
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

uint64_t strlen(const char *s)
{
    uint64_t len = 0;

    while (s[len])
        len++;

    return len;
}

char *strrchr(const char *str, int c)
{
    const char *last = 0;

    while (*str)
    {
        if (*str == (char)c)
            last = str;

        str++;
    }

    // Handle searching for '\0'
    if (c == 0)
        return (char *)str;

    return (char *)last;
}

int abs(int x)
{
    if (x < 0)
        return -x;

    return x;
}

// For more randomness use 8:th not 1:th
char e_digit(uint64_t x) {
    uint64_t div = 1;

    // Find the highest power of 10.
    while (x / div >= 10) {
        div *= 10;
    }

    // Skip the first 7 digits.
    for (int i = 0; i < 7 && div > 1; i++) {
        div /= 10;
    }

    return (char)('0' + ((x / div) % 10));
}

void cli(void)
{
    __asm__ volatile ("cli" ::: "memory");
}

void sti(void)
{
    __asm__ volatile ("sti" ::: "memory");
}

void *kmemmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d == s || n == 0)
        return dest;

    if (d < s) {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        for (size_t i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }

    return dest;
}
