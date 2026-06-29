#pragma once

#include <stdint.h>
#include <stddef.h>
#include <drivers/task/main.h>

static uint8_t *heap_start = (uint8_t*)0x100000;
static size_t heap_offset = 0;

void kmalloc_init(uint8_t *start)
{
    heap_start = start;
    heap_offset = 0;
}

void kmalloc_deinit(cs_task *self)
{
    (void)self;

    heap_start = NULL;
    heap_offset = 0;
}

// LIFO order only

void *kmalloc(size_t size)
{
    size = (size + 15) & ~15; // align 16 bytes

    void *ptr = heap_start + heap_offset;
    heap_offset += size;

    return ptr;
}

void kfree(size_t size)
{
    size = (size + 15) & ~15;

    if (size > heap_offset)
        heap_offset = 0;
    else
        heap_offset -= size;
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
