// device.h
#ifndef DEVICE_H
#define DEVICE_H

int device_open(const char *device);
void device_close(int fd, const char *device);

#endif
