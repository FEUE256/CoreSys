// writer.c
#include "writer.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

bool writer_write_image(int fd, const char *image, volatile sig_atomic_t *stop_flag) {
    FILE *fp = fopen(image, "rb");
    if (!fp) {
        perror("[writer] Failed to open image file");
        return false;
    }

    struct stat st;
    if (stat(image, &st) != 0) st.st_size = 0;
    off_t total_size = st.st_size;
    off_t written_size = 0;

    printf("[writer] Writing image %s\n", image);

    char buffer[65536]; // 64 KiB buffert
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (*stop_flag) {
            fclose(fp);
            fprintf(stderr, "[writer] Aborted by user\n");
            return false;
        }

        size_t total = 0;
        while (total < bytes) {
            ssize_t written = write(fd, buffer + total, bytes - total);
            if (written <= 0) {
                perror("[writer] Failed to write to device");
                fclose(fp);
                return false;
            }
            total += written;
            written_size += written;

            if (total_size > 0) {
                printf("\rProgress: %.2f%%", (double)written_size / total_size * 100);
                fflush(stdout);
            }
        }
    }

    printf("\n");

    if (fsync(fd) < 0) {
        perror("[writer] fsync failed");
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}
