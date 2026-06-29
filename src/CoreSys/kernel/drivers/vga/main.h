#pragma once

// Thanks to dev.to and https://dev.to/frosnerd/writing-my-own-vga-driver-22nn and https://github.com/FRosner/FrOS

unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

#define VGA_CTRL_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_OFFSET_LOW 0x0f
#define VGA_OFFSET_HIGH 0x0e

void set_cursor(int offset) {
    offset /= 2;
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char) (offset >> 8));
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char) (offset & 0xff));
}

int get_cursor() {
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    int offset = port_byte_in(VGA_DATA_REGISTER) << 8;
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    offset += port_byte_in(VGA_DATA_REGISTER);
    return offset * 2;
}

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f

void set_char_at_video_memory(char character, int offset) {
    unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;
    vidmem[offset] = character;
    vidmem[offset + 1] = WHITE_ON_BLACK;
}

int get_row_from_offset(int offset) {
    return offset / (2 * MAX_COLS);
}

int get_offset(int col, int row) {
    return 2 * (row * MAX_COLS + col);
}

int move_offset_to_new_line(int offset) {
    return get_offset(0, get_row_from_offset(offset) + 1);
}

void print_string(char *string) {
    int offset = get_cursor();
    int i = 0;
    while (string[i] != 0) {
        if (string[i] == '\n') {
            offset = move_offset_to_new_line(offset);
        } else {
            set_char_at_video_memory(string[i], offset);
            offset += 2;
        }
        i++;
    }
    set_cursor(offset);
}

void clear_screen() {
    for (int i = 0; i < MAX_COLS * MAX_ROWS; ++i) {
        set_char_at_video_memory(' ', i * 2);
    }
    set_cursor(get_offset(0, 0));
}
// Thanks to popcorn bootloader and kernel github

#include <stdint.h>

// Marked as volatile so that the compiler doesn't optimize it out.
volatile uint16_t *video_memory = (uint16_t *)0xb8000;
int cursor = 0;

void *vga_reverse(char *arr, int s, int e) {
    char tmp;

    while (s < e) {
        tmp = arr[s];
        arr[s] = arr[e];
        arr[e] = tmp;
        s++;
        e--;
    }

    return arr;
}

void vga_write_char(char c) {
  const int color = 0x0f00;
  video_memory[cursor] = color | c;
  ++cursor;
}

void vga_print(char *str) {
  uint16_t i = 0;
  while (str[i] != 0x0 && i < 80) {
    vga_write_char(str[i]);
    i++;
  }
}

void vga_println(char *str) {
  vga_print(str);
  cursor += 80 - cursor % 80;
}

void vga_clear() {
  for (int i = 0; i < 80 * 25; i++) {
    video_memory[i] = 0x0f00 | ' ';
  }
  cursor = 0;
}