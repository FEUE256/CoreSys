#pragma once

#include <stdint.h>
#include <stddef.h>

static uint8_t *heap_start = (uint8_t*)0x100000;
static size_t heap_offset = 0;

void kmalloc_init(uint8_t *start)
{
    heap_start = start;
    heap_offset = 0;
}

void *kmalloc(size_t size)
{
    size = (size + 15) & ~15; // align 16 bytes

    void *ptr = heap_start + heap_offset;
    heap_offset += size;

    return ptr;
}

void kfree(void *ptr)
{
    (void)ptr; // no-op (bump allocator)
}

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = dest;
    const uint8_t *s = src;

    for (size_t i = 0; i < n; i++)
        d[i] = s[i];

    return dest;
}

void *memset(void *dest, int val, size_t n)
{
    uint8_t *d = dest;

    for (size_t i = 0; i < n; i++)
        d[i] = (uint8_t)val;

    return dest;
}
