// #pragma once

// #include <stdint.h>
// #include <stddef.h>
// #include <drivers/task/main.h>

// static uint8_t *heap_start = (uint8_t*)0x100000;
// static size_t heap_offset = 0;

// void kmalloc_init(uint8_t *start)
// {
//     heap_start = start;
//     heap_offset = 0;
// }

// void kmalloc_deinit(cs_task *self)
// {
//     (void)self;

//     heap_start = NULL;
//     heap_offset = 0;
// }

// // LIFO order only

// void *kmalloc(size_t size)
// {
//     size = (size + 15) & ~15; // align 16 bytes

//     void *ptr = heap_start + heap_offset;
//     heap_offset += size;

//     return ptr;
// }

// void kfree(size_t size)
// {
//     size = (size + 15) & ~15;

//     if (size > heap_offset)
//         heap_offset = 0;
//     else
//         heap_offset -= size;
// }

#pragma once
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PMM_MAX_MEMORY (4ULL * 1024 * 1024 * 1024)
#define PMM_MAX_PAGES (PMM_MAX_MEMORY / PAGE_SIZE)

static uint8_t pmm_bitmap[PMM_MAX_PAGES / 8];

static uint64_t pmm_base = 0;
static size_t pmm_total_pages = 0;

static inline void pmm_set(size_t page)
{
    pmm_bitmap[page >> 3] |=
        (1 << (page & 7));
}

static inline void pmm_clear(size_t page)
{
    pmm_bitmap[page >> 3] &=
        ~(1 << (page & 7));
}

static inline int pmm_used(size_t page)
{
    return
        (pmm_bitmap[page >> 3]
        >> (page & 7)) & 1;
}

void pmm_init(
    uint64_t usable_start,
    uint64_t usable_size
)
{
    pmm_base =
        usable_start;

    pmm_total_pages =
        usable_size /
        PAGE_SIZE;

    for (size_t i = 0;
         i < PMM_MAX_PAGES / 8;
         i++)
    {
        pmm_bitmap[i] = 0xFF;
    }

    for (size_t i = 0;
         i < pmm_total_pages;
         i++)
    {
        pmm_clear(i);
    }
}

void* pmm_alloc_pages(
    size_t pages
)
{
    if (!pages)
        return NULL;

    size_t run = 0;
    size_t start = 0;

    for (size_t i = 0;
         i < pmm_total_pages;
         i++)
    {
        if (!pmm_used(i))
        {
            if (!run)
                start = i;

            run++;

            if (run == pages)
            {
                for (size_t j = start;
                     j < start + pages;
                     j++)
                {
                    pmm_set(j);
                }

                return
                    (void*)
                    (
                        pmm_base +
                        start *
                        PAGE_SIZE
                    );
            }
        }
        else
        {
            run = 0;
        }
    }

    return NULL;
}

void pmm_free_pages(
    void* ptr,
    size_t pages
)
{
    if (!ptr || !pages)
        return;

    size_t page =
        (
            (
                uint64_t)ptr -
            pmm_base
        ) /
        PAGE_SIZE;

    for (size_t i = 0;
         i < pages;
         i++)
    {
        pmm_clear(
            page + i
        );
    }
}

typedef struct
{
    size_t pages;
} kmalloc_hdr;

/*
Required:
void* pmm_alloc_pages(size_t pages);
void  pmm_free_pages(void* ptr, size_t pages);
*/

static inline size_t kmalloc_align(size_t size)
{
    return (size + 4095) / 4096;
}

void* kmalloc(size_t size)
{
    if (size == 0)
        return NULL;

    size_t total = size + sizeof(kmalloc_hdr);
    size_t pages = kmalloc_align(total);

    kmalloc_hdr* hdr = (kmalloc_hdr*)pmm_alloc_pages(pages);
    if (!hdr)
        return NULL;

    hdr->pages = pages;

    return (void*)(hdr + 1);
}

void kfree(void* ptr)
{
    if (!ptr)
        return;

    kmalloc_hdr* hdr =
        ((kmalloc_hdr*)ptr) - 1;

    pmm_free_pages(
        (void*)hdr,
        hdr->pages
    );
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

static inline uint64_t virt_to_phys(void* addr)
{
    return (uint64_t)addr;
}
