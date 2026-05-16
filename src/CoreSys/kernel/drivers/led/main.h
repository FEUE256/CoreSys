#define KBD_STATUS 0x64
#define KBD_DATA   0x60

static void kbd_wait_input(void)
{
    while (inb(KBD_STATUS) & 0x02); // wait input buffer clear
}

void set_keyboard_leds(uint8_t flags)
{
    // 0xED = Set LEDs command
    kbd_wait_input();
    outb(KBD_DATA, 0xED);

    kbd_wait_input();
    outb(KBD_DATA, flags); // bit0=Scroll, bit1=Num, bit2=Caps
}

void led_demo()
{
    while (1)
    {
        set_keyboard_leds(0x04); // Caps ON
        for (volatile int i = 0; i < 800000; i++);

        set_keyboard_leds(0x00); // All OFF
        for (volatile int i = 0; i < 800000; i++);
    }
}
