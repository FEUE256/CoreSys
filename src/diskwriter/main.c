// main.c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "device.h"
#include "writer.h"
#include "verify.h"

volatile sig_atomic_t stop_flag = 0;

void handle_sigint(int sig) {
    (void)sig;
    stop_flag = 1;
}

int print_usage(const char *prog_name) {
    printf("Usage: sudo %s <source_image> <target_device>\n", prog_name);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) return print_usage(argv[0]);

    const char *source_image = argv[1];
    const char *target_device = argv[2];

    signal(SIGINT, handle_sigint);

    int fd = device_open(target_device);
    if (fd < 0) return EXIT_FAILURE;

    if (!writer_write_image(fd, source_image, &stop_flag)) {
        device_close(fd, target_device);
        fprintf(stderr, "[DiskWriter] Failed writing image\n");
        return EXIT_FAILURE;
    }

    if (!verify_image(fd, source_image, &stop_flag)) {
        device_close(fd, target_device);
        fprintf(stderr, "[DiskWriter] Verification failed\n");
        return EXIT_FAILURE;
    }

    device_close(fd, target_device);
    printf("[DiskWriter] Completed successfully!\n");
    return EXIT_SUCCESS;
}
