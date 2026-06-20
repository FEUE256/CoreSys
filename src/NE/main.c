#include <main.h>

int main(int argc, char *argv[]) {
    const char *input = NULL;
    const char *output = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = argv[++i];
        }
    }

    if (!input || !output) {
        fprintf(stderr, "Usage: %s -i <input> -o <output>\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(input, "rb");
    if (!fin) {
        perror("Input open failed");
        return 1;
    }

    // Get input file size
    fseek(fin, 0, SEEK_END);
    long size = ftell(fin);
    rewind(fin);

    if (size < 0) {
        fprintf(stderr, "Failed to determine file size\n");
        fclose(fin);
        return 1;
    }

    // Allocate buffer
    unsigned char *data = malloc(size);
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fin);
        return 1;
    }

    // Read entire file
    if (fread(data, 1, size, fin) != (size_t)size) {
        fprintf(stderr, "Failed to read input file\n");
        free(data);
        fclose(fin);
        return 1;
    }

    fclose(fin);

    FILE *fout = fopen(output, "wb");
    if (!fout) {
        perror("Output open failed");
        free(data);
        return 1;
    }

    // Write "NE"
    fwrite("NE", 1, 2, fout);

    // Write original content AFTER
    fwrite(data, 1, size, fout);

    fclose(fout);
    free(data);

    printf("Prepended NE header successfully.\n");
    return 0;
}
