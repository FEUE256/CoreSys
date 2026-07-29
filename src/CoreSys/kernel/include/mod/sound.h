#include <stdint.h>

#define PIT_FREQ 1193182
#define PC_SPEAKER 0x61

#include <drivers/serial/main.h>
#include <drivers/task/main.h>

static inline void delay(uint32_t ms)
{
    volatile uint64_t count = ms * 10000;

    while (count--)
    {
        __asm__ volatile ("pause");
    }
}

void speaker_on(uint32_t frequency)
{
    uint32_t divisor = PIT_FREQ / frequency;

    outb(0x43, 0xB6);
    outb(0x42, divisor & 0xFF);
    outb(0x42, (divisor >> 8) & 0xFF);

    uint8_t tmp = inb(PC_SPEAKER);
    if (tmp != (tmp | 3))
        outb(PC_SPEAKER, tmp | 3);
}

void speaker_off()
{
    uint8_t tmp = inb(PC_SPEAKER);
    outb(PC_SPEAKER, tmp & ~3);
}

void boot_sound(cs_task *self)
{
    (void)self;
    
    speaker_on(523); // C5
    delay(100);

    speaker_off();

    speaker_on(659); // E5
    delay(100);

    speaker_off();

    speaker_on(784); // G5
    delay(200);

    speaker_off();
}
