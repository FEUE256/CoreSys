#pragma once

#include <stdint.h>

void tty_putc(char c);
void tty_write(const char *s);
void tty_loop(int debug);
void serial_write_char(char c);
char serial_read_char();
void execute_command(const char *cmd, int debug);
void kshutdown(cs_task *self);
void kreboot(cs_task *self);

#include <drivers/hal/main.h>

#include <drivers/ACPI/main.h>
#include <drivers/led/main.h>
#include <drivers/serial/main.h>
#include <drivers/time/main.h>
#include <drivers/tty/main.h>
#include <drivers/tty/cmd.h>
#include <drivers/ret/main.h>
#include <drivers/task/main.h>

// Dispatcher
uint64_t halcall(hal_frame_t* frame)
{
    uint64_t id = frame->rax;

    if (id == HAL_DEV_NULL) {
        return (int)ret(frame->rdi);
    }
    else if (id == HAL_EXECUTE_COMMAND) {
        execute_command((const char*)frame->rdi, (int)frame->rsi);
        return 0;
    }
    else if (id == HAL_TTY_LOOP) {
        tty_loop((int)frame->rdi);
        return 0;
    }
    else if (id == HAL_TTY_WRITE) {
        tty_write((const char*)frame->rdi);
        return 0;
    }
    else if (id == HAL_TTY_PUTC) {
        tty_putc((char)frame->rdi);
        return 0;
    }
    else if (id == HAL_OUTB) {
        outb((u16)frame->rdi, (u8)frame->rsi);
        return 0;
    }
    else if (id == HAL_INB) {
        return (uint64_t)inb((u16)frame->rdi);
    }
    else if (id == HAL_OUTW) {
        outw((uint16_t)frame->rdi, (uint16_t)frame->rsi);
        return 0;
    }
    else if (id == HAL_INW) {
        return (uint64_t)inw((uint16_t)frame->rdi);
    }
    else if (id == HAL_SERIAL_TX_EMPTY) {
        return (uint64_t)serial_is_transmit_empty();
    }
    else if (id == HAL_SERIAL_RECEIVED) {
        return (uint64_t)serial_received();
    }
    else if (id == HAL_SERIAL_WRITE_CHAR) {
        serial_write_char((char)frame->rdi);
        return 0;
    }
    else if (id == HAL_A_CHAR_PRINT) {
        a_char_print((char)frame->rdi);
        return 0;
    }
    else if (id == HAL_SERIAL_WRITE) {
        serial_write((const char*)frame->rdi);
        return 0;
    }
    else if (id == HAL_SERIAL_WRITE_U64) {
        serial_write_u64(frame->rdi);
        return 0;
    }
    else if (id == HAL_SERIAL_WRITE_PTR) {
        serial_write_ptr((const void*)frame->rdi);
        return 0;
    }
    else if (id == HAL_KPRINT) {
        kprint((const char*)frame->rdi);
        return 0;
    }
    else if (id == HAL_KPRINT_CHAR) {
        kprint_char((char)frame->rdi);
        return 0;
    }
    else if (id == HAL_KPRINT_U64) {
        kprint_u64(frame->rdi);
        return 0;
    }
    else if (id == HAL_KPRINT_U8) {
        kprint_u8((uint8_t)frame->rdi);
        return 0;
    }
    else if (id == HAL_KPRINTF) {
        kprintf((const char*)frame->rdi);
        return 0;
    }
    else if (id == HAL_SERIAL_READ_CHAR) {
        return (uint64_t)serial_read_char();
    }
    else if (id == HAL_KREAD) {
        return (uint64_t)kread();
    }
    else if (id == HAL_SERIAL_CLEAR) {
        serial_clear(NULL);
        return 0;
    }
    else if (id == HAL_KCLEAR) {
        kclear();
        return 0;
    }
    else if (id == HAL_KCLEAR) {
        kclear();
        return 0;
    }
    else if (id == HAL_K_CLEAR) {
        kclear();
        return 0;
    }
    else if (id == HAL_KSTRLEN) {
        return (uint64_t)kstrlen((const char*)frame->rdi);
    }
    else if (id == HAL_KPRINT_STR) {
        kprint_str((const char*)frame->rdi);
        return 0;
    } 
    else if (id == HAL_KPRINT_UINT) {
        kprint_uint((unsigned long long)frame->rdi, (unsigned)frame->rsi);
        return 0;
    }
    else if (id == HAL_KPRINT_INT) {
        kprint_int((long long)frame->rdi);
        return 0;
    }
    else if (id == HAL_INIT_SERIAL) {
        initSerial(NULL);
        return 0;
    }
    else if (id == HAL_DEINIT_SERIAL) {
        deinitSerial(NULL);
        return 0;
    }
    else if (id == HAL_LED_DEMO) {
        led_demo();
        return 0;
    }
    else if (id == HAL_SHUTDOWN) {
        cs_task kshutdown_task = {
            .name = "Kshutdown Task",
            .source_header = "drivers/ACPI/main.h",
            .entry = kshutdown
        };
        task_run(&kshutdown_task);
        return 0;
    }
    else if (id == HAL_REBOOT) {
        cs_task kreboot_task = {
            .name = "Kreboot Task",
            .source_header = "drivers/ACPI/main.h",
            .entry = kreboot
        };
        task_run(&kreboot_task);
        return 0;
    }
    else {
         __asm__ volatile (".word 0xFFFF"); // #UD for invalid syscall
    }   

    return (uint64_t)-1;
}
