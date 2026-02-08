// verify.c
#include "verify.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

bool verify_image(int fd, const char *image, volatile sig_atomic_t *stop_flag) {
    FILE *fp = fopen(image, "rb");
    if (!fp) {
        perror("[verify] Failed to open image");
        return false;
    }

    struct stat st;
    if (stat(image, &st) != 0) st.st_size = 0;
    off_t total_size = st.st_size;
    off_t read_size = 0;

    printf("[verify] Verifying image %s\n", image);

    char buf_image[65536];
    char buf_dev[65536];
    size_t bytes;

    lseek(fd, 0, SEEK_SET);

    while ((bytes = fread(buf_image, 1, sizeof(buf_image), fp)) > 0) {
        if (*stop_flag) {
            fclose(fp);
            fprintf(stderr, "[verify] Aborted by user\n");
            return false;
        }

        size_t total = 0;
        while (total < bytes) {
            ssize_t r = read(fd, buf_dev + total, bytes - total);
            if (r <= 0) {
                perror("[verify] Read error");
                fclose(fp);
                return false;
            }
            total += r;
            read_size += r;

            if (total_size > 0) {
                printf("\rProgress: %.2f%%", (double)read_size / total_size * 100);
                fflush(stdout);
            }
        }

        if (memcmp(buf_image, buf_dev, bytes) != 0) {
            printf("\n[verify] Mismatch detected\n");
            fclose(fp);
            return false;
        }
    }

    printf("\n");
    fclose(fp);
    return true;
}
