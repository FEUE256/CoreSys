#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

extern char **environ;
#define _GNU_SOURCE
void kill_parent();
#define MAX_ARGS 32

int main() {
    // Buffer that stores user input command line
    char command[256];

    // Print initial help hint to user once at startup
    (void)write(1, "Type 'help' for help\n", 21);

    // Infinite shell loop (keeps CoreSys shell running)
    for (;;) {

        // Print shell prompt
        (void)write(1, "CoreSys Linux # ", 16);

        // Read user input from stdin into command buffer
        int n = read(0, command, sizeof(command) - 1);

        // If read fails or nothing is entered, restart loop
        if (n <= 0) continue;

        // Remove newline from input or terminate string safely
        if (command[n - 1] == '\n')
            command[n - 1] = '\0';
        else
            command[n] = '\0';

        // Array of argument pointers for execve
        char *argv[MAX_ARGS];
        int argc = 0;

        // Pointer used for parsing the input string
        char *p = command;

        // Tokenize input string into arguments separated by spaces
        while (*p) {

            // Skip leading spaces
            while (*p == ' ') p++;

            // Stop if end of string is reached
            if (*p == '\0') break;

            // Store current argument start position
            argv[argc++] = p;

            // Move pointer until next space or end
            while (*p && *p != ' ') p++;

            // Replace space with null terminator to split string
            if (*p) {
                *p = '\0';
                p++;
            }

            // Prevent overflow of argument array
            if (argc >= MAX_ARGS - 1) break;
        }

        // Null terminate argument list (required by execve)
        argv[argc] = NULL;

        // If no command was entered, restart loop
        if (argc == 0) continue;

        // Create new process (child) for executing command
        pid_t pid = fork();

        if (pid == 0) {
            if (strcmp(command, "help") == 0) {
                (void)write(1, "CoreSys Linux, Run help to get this message, Always use Absolute path execution, write /busybox sh to start a Buzy box shell with config /.config/busybox.cfg\n", 81);
            } else if (strcmp(command, "exit") == 0) {
                break;
                kill_parent();
            } else {
                // Child process: execute command replacing current process image
                char path[128];

                if (argv[0][0] == '/') {
                    execve(argv[0], argv, environ);
                } else {
                    snprintf(path, sizeof(path), "/%s", argv[0]);
                    execve(path, argv, environ);
                }

                // If execve fails, print error message
                const char msg[] = "execve failed\n";
                (void)write(1, msg, sizeof(msg) - 1);

                // Exit child process immediately
                _exit(1);
            }
        }

        if (pid > 0) {
            // Parent process: wait for child to finish execution
            waitpid(pid, NULL, 0);
        }
    }
}
