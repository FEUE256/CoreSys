// device.c
#include "device.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int device_open(const char *device) {
    int fd = open(device, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("[device] Failed to open device");
        return -1;
    }
    printf("[device] Opened device %s\n", device);
    return fd;
}

void device_close(int fd, const char *device) {
    if (fd >= 0) {
        close(fd);
        printf("[device] Closed device %s\n", device);
    }
}
