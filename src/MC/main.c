#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for access()

char* readFile(const char* path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 0) {
        fclose(file);
        return NULL;
    }

    char *buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    if (fread(buffer, 1, size, file) != (size_t)size) {
        fclose(file);
        free(buffer);
        fprintf(stderr, "Error: Failed to read entire file\n");
        return NULL;
    }

    buffer[size] = '\0';
    fclose(file);
    return buffer;
}

char* bt(const char* binary) {
    size_t len = strlen(binary);
    if (len % 8 != 0) return NULL;

    size_t text_len = len / 8;
    char* text = malloc(text_len + 1);
    if (!text) return NULL;

    for (size_t i = 0; i < text_len; i++) {
        char byte_str[9];
        strncpy(byte_str, &binary[i * 8], 8);
        byte_str[8] = '\0';

        char c = (char)strtol(byte_str, NULL, 2);
        text[i] = (strcmp(byte_str, "00001010") == 0) ? '\n' : c;
    }
    text[text_len] = '\0';
    return text;
}

int writeFile(const char* data, const char* path) {
    if (!data || !path) return -1;
    FILE* fp = fopen(path, "w");
    if (!fp) {
        perror("Failed to open file");
        return -1;
    }
    if (fputs(data, fp) == EOF) {
        perror("Failed to write to file");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

int deleteFile(const char* path) {
    if (!path) return -1;
    if (remove(path) == 0) return 0;
    perror("Failed to delete file");
    return -1;
}

int shell(const char* command) {
    if (!command) return -1;
    int result = system(command);
    if (result == -1) perror("Failed to run shell command");
    return result;
}

void checkAndInstallNASM() {
    int status = shell("command -v nasm > /dev/null 2>&1");
    if (status != 0) {
        printf("NASM not found. Installing...\n");
        status = shell("sudo apt update && sudo apt install -y nasm");
        printf(status == 0 ? "NASM installed successfully.\n" : "Failed to install NASM.\n");
    } else {
        printf("NASM is already installed.\n");
    }
}

void checkAndInstallLD() {
    int status = shell("command -v ld > /dev/null 2>&1");
    if (status != 0) {
        printf("ld (GNU linker) not found. Installing...\n");
        status = shell("sudo apt update && sudo apt install -y binutils");
        printf(status == 0 ? "ld installed successfully.\n" : "Failed to install ld.\n");
    } else {
        printf("ld (GNU linker) is already installed.\n");
    }
}

void remove_chars(const char *path, const char *target) {
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read entire file into memory
    char *buffer = (char *)malloc(filesize + 1);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }
    size_t read_bytes = fread(buffer, 1, filesize, file);
    if (read_bytes != filesize) {
        perror("Failed to read full file");
        fclose(file);
        free(buffer);
        return;
    }
    buffer[filesize] = '\0';
    fclose(file);

    // Create temporary buffer for cleaned text
    char *cleaned = (char *)malloc(filesize + 1);
    if (!cleaned) {
        perror("Memory allocation failed");
        free(buffer);
        return;
    }

    char *p = buffer;
    char *q = cleaned;
    size_t target_len = strlen(target);

    while (*p) {
        if (strncmp(p, target, target_len) == 0) {
            p += target_len; // skip "00001010"
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';

    // Write cleaned content back to file
    file = fopen(path, "w");
    if (!file) {
        perror("Failed to write to file");
        free(buffer);
        free(cleaned);
        return;
    }
    fwrite(cleaned, 1, strlen(cleaned), file);
    fclose(file);

    free(buffer);
    free(cleaned);
}

int main(int argc, char *argv[]) {
    char *inputFile = NULL;
    char *outputFile = NULL;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-i") == 0 && i + 1 < argc) inputFile = argv[++i];
        else if(strcmp(argv[i], "-o") == 0 && i + 1 < argc) outputFile = argv[++i];
    }

    if(!inputFile || !outputFile) {
        fprintf(stderr, "Usage: %s -i input.mc -o output.run\n", argv[0]);
        return 1;
    }

    printf("NOTE: In the .mc file do not use newfiles of spaces or anything that not is a 0 or a 1!\n");

    if(access(inputFile, F_OK) != 0) {
        fprintf(stderr, "Error: Input file '%s' does not exist.\n", inputFile);
        return 1;
    }

    char *inputContent = readFile(inputFile);
    if (!inputContent) {
        fprintf(stderr, "Error: Failed to read input file.\n");
        return 1;
    }

    char *binaryContent = bt(inputContent);
    if (!binaryContent) {
        fprintf(stderr, "Error: Failed to convert binary content.\n");
        return 1;
    }

    writeFile(binaryContent, "data.asm"); // use .asm extension
    remove_chars("data.asm", "00001010"); // remove "00001010" from the .asm file
    checkAndInstallNASM();
    checkAndInstallLD();

    printf("DEBUG binaryContent var: %s\n", binaryContent); // Debug output of the binary content
    printf("DEBUG inputContent var: %s\n", inputContent); // Debug output of the original input content

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "nasm -f elf64 data.asm -o data.o");
    shell(cmd);
    snprintf(cmd, sizeof(cmd), "ld data.o -o %s", outputFile);
    shell(cmd);

    deleteFile("data.asm");
    deleteFile("data.o");
    free(binaryContent);
    free(inputContent);
    return 0;
}
