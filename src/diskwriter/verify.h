// verify.h
#ifndef VERIFY_H
#define VERIFY_H

#include <stdbool.h>
#include <signal.h> // for sig_atomic_t

bool verify_image(int fd, const char *image, volatile sig_atomic_t *stop_flag);

#endif
