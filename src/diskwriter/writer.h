// writer.h
#ifndef WRITER_H
#define WRITER_H

#include <stdbool.h>
#include <signal.h> // Add this for sig_atomic_t

bool writer_write_image(int fd, const char *image, volatile sig_atomic_t *stop_flag);

#endif
