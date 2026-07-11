#pragma once

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static uint8_t cmos_read(uint8_t reg)
{
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

uint8_t get_seconds(void)
{
    return cmos_read(0x00);
}

uint8_t get_minutes(void)
{
    return cmos_read(0x02);
}

uint8_t get_hours(void)
{
    return cmos_read(0x04);
}

static inline uint8_t bcd_to_bin(uint8_t v)
{
    return (v & 0x0F) + ((v >> 4) * 10);
}

void format_time(char *out, uint8_t h, uint8_t m, uint8_t s)
{
    out[0] = '0' + (h / 10);
    out[1] = '0' + (h % 10);
    out[2] = ':';

    out[3] = '0' + (m / 10);
    out[4] = '0' + (m % 10);
    out[5] = ':';

    out[6] = '0' + (s / 10);
    out[7] = '0' + (s % 10);

    out[8] = 0;
}

void format_regb(char *out, uint8_t regB)
{
    const char hex[] = "0123456789ABCDEF";
 
    out[0] = 0;
    out[0] = hex[(regB >> 4) & 0x0F];
    out[1] = hex[regB & 0x0F];
    out[2] = 0;
}

void get_time() {
    uint8_t sec_bcd  = cmos_read(0x00);
    uint8_t min_bcd  = cmos_read(0x02);
    uint8_t hour_bcd = cmos_read(0x04);

    uint8_t regB = cmos_read(0x0B);

    if (!(regB & 0x04)) // not binary mode → BCD mode
    {
        sec_bcd  = bcd_to_bin(sec_bcd);
        min_bcd  = bcd_to_bin(min_bcd);
        hour_bcd = bcd_to_bin(hour_bcd);
    }

    char time[9];

    format_time(time, hour_bcd, min_bcd, sec_bcd);
    kprint(time);
    kprint("\n");
}

void get_regb() {
    uint8_t regB = cmos_read(0x0B);
    char buf[3];
    format_regb(buf, regB);
    kprint(buf);
    kprint("\n");
}
