#pragma once

#include <core/efi.h>
#include <drivers/tty/main.h>
#include <drivers/halt/main.h>
#include <drivers/ACPI/main.h>
#include <drivers/sf/main.h>
#include <drivers/log/main.h>
#include <kernel/version.h>
#include <drivers/page/main.h>
#include <drivers/task/main.h>
#include <drivers/sys/main.h>
#include <drivers/syscalls/main.h>
#include <drivers/cfs/main.h>
#include <drivers/led/main.h>
#include <drivers/reg/main.h>
#include <drivers/ata/main.h>
#include <stddef.h>
#include <stdint.h>

void sys_write(const char* s);
void sys_halt(void);
char sys_read(void);
void sys_init(void);
void sys_clear(void);
extern void tty_loop();
extern void tty_write(const char *s);
extern void k_sf(const char *s);

int syscall_test_main(void) {
    return 0;
}

void execute_command(const char *cmd, int debug)
{
    // HELP
    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p')
    {
        tty_write(
            "Commands:\n"
            " help      - Show help\n"
            " clear     - Clear terminal\n"
            " cls       - Alias for clear\n"
            " echo      - Print text\n"
            " shutdown  - Shut down system\n"
            " reboot    - Reboot system\n"
            " hlt       - Halt CPU\n"
            " sf        - System failure test\n"
            " ver       - Show version\n"
            " fsinfo    - Filesystem info\n"
            " ps        - List processes\n"
            " ii        - ASCII table\n"
            " rax       - Print RAX\n"
            " run       - Run syscall test\n"
            " cfg       - Show kernel config\n"
            " cr        - Show credits\n"
            " led       - Keyboard LED demo\n"
            " ata       - ATA dump\n"
            " debug     - Show debug value\n"
            " reg       - Register dump\n"
        );
    }
    // CLEAR / CLS
    else if (
        (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r') ||
        (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 's')
    )
    {
        tty_write("\x1b[2J\x1b[H");
    }
    // ECHO
    else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o')
    {
        // skip "echo" + optional space
        const char *p = cmd + 4;
        if (*p == ' ') p++;

        tty_write(p);
        tty_write("\n");
    }
    else if (cmd[0] == 's' && cmd[1] == 'h' && cmd[2] == 'u' && cmd[3] == 't' &&
         cmd[4] == 'd' && cmd[5] == 'o' && cmd[6] == 'w' && cmd[7] == 'n') {
            shutdown();
         }
    else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'b' && cmd[3] == 'o' &&
         cmd[4] == 'o' && cmd[5] == 't') {
            reboot();
         }
    else if (cmd[0] == 'h' && cmd[1] == 'l' && cmd[2] == 't')
    {
        cs_task hlt_task = {
            .name = "Halt Task",
            .source_header = "drivers/halt/main.h",
            .entry = hlt
        };
        task_run(&hlt_task); // Halt the system
    }
    else if (cmd[0] == 's' && cmd[1] == 'f')
    {
        k_sf("Tested System Failure via CMD"); // TEST 
    }
    else if (cmd[0] == 'v' && cmd[1] == 'e' && cmd[2] == 'r') {
        kprint("FÈUE CoreSys Kernel Version: ");
        kprint(CS_VER);
        kprint("\r\n");
    }
    else if (cmd[0] == 'f' && cmd[1] == 's' && cmd[2] == 'i' && cmd[3] == 'n' && cmd[4] == 'f' && cmd[5] == 'o') {
        kprint("Filesystem: cfs\n");
    }
    else if (cmd[0] == 'p' && cmd[1] == 's')
    {
        kprint("List of proccesses:\n");
        kprint("KERNEL.RE\n");
        kprint("    Terminal (CMD)\n");
        kprint("        ps (CMD)\n");
    }
    else if (cmd[0] == 'r' && cmd[1] == 'a' && cmd[2] == 'x')
    {
        uint64_t rax_value;

        __asm__ volatile (
            "mov %%rax, %0"
            : "=r"(rax_value)
        );

        serial_write_u64(rax_value);
        kprint("\n");
    }
    else if (cmd[0] == 'r' && cmd[1] == 'u' && cmd[2] == 'n')
    {
        syscall_test_main();
    }
    else if (cmd[0] == 'c' && cmd[1] == 'f' && cmd[2] == 'g')
    {
        uint64_t size = 0;
        uint8_t* kernel_cfg = cfs_read(kernel_cfg_file, &size);

        if (kernel_cfg == NULL) 
        {
            sys_write("Kernel Configuration: <kc_null_invalid>\n");
            return;
        }

        if (!kernel_cfg)
        {
            sys_write("Kernel Configuration: <no_kc_invalid>\n");
            return;
        }

        if (size == 0)
        {
            sys_write("Kernel Configuration: <size_z_invalid>");
        }

        char buffer[4092];

        if (size >= sizeof(buffer))
            size = sizeof(buffer) - 1;

        memcpy(buffer, kernel_cfg, size);
        buffer[size] = '\0';

        sys_write(buffer);
        sys_write("\n");
    }
    else if (cmd[0] == 'c' && cmd[1] == 'r')
    {
        kprint("Thanks to Queso Fuego (parts of code), elevatorguy (parts of code), Cyber::Boot (fork), Terry A. Davis (TempleOS), Ankit Kumar (Polaris OS), Neptune650 (Polaris OS), MishaTy (Polaris OS), redmine4404 (Polaris OS), AnalogFeelings (Polaris), 1010101001010101 (tinycrypt), chris-morrison (tinycrypt), mczraf (tinycrypt), Ipereira (tinycrypt), malsbat (tinycrypt), rob-brown (tinycrypt), haukepetersen (tinycrypt), mped-oticon (tinycrypt), thoh-ot (tinycrypt), daor-oti (tinycrypt), winnietwo (tinycrypt), sfblackl-intel (tinycrypt), every person on the EDK II team and many more for the inspiration and help in making this project possible! RIP Terry A. Davis!\r\n");
    }
    else if (cmd[0] == 'l' && cmd[1] == 'e' && cmd[2] == 'd') {
        led_demo();
    }
    else if (cmd[0] == 'a' && cmd[1] == 't' && cmd[2] == 'a') {
        hexdump_512_all();
    }
    else if (cmd[0] == 'd' && cmd[1] == 'e' && cmd[2] == 'b' && cmd[3] == 'u' && cmd[4] == 'g') {
        kprintf("%d\n", debug);
    }
    else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'g') {
        if (debug == 1) {
            print_regs();
        } else {
            k_warning("Enable Debug mode for this feature\n");
        }
    }
    else if (cmd[0] == 'i' && cmd[1] == 'i')
    {
        kprint("ASCII TABLE (0–127)\n");

        kprint("Dec  Hex  Char\n");
        kprint("----------------\n");

        kprint("  0   00   NUL\n");
        kprint("  1   01   SOH\n");
        kprint("  2   02   STX\n");
        kprint("  3   03   ETX\n");
        kprint("  4   04   EOT\n");
        kprint("  5   05   ENQ\n");
        kprint("  6   06   ACK\n");
        kprint("  7   07   BEL\n");
        kprint("  8   08   BS\n");
        kprint("  9   09   TAB\n");
        kprint(" 10   0A   LF\n");
        kprint(" 11   0B   VT\n");
        kprint(" 12   0C   FF\n");
        kprint(" 13   0D   CR\n");
        kprint(" 14   0E   SO\n");
        kprint(" 15   0F   SI\n");

        kprint(" 16   10   DLE\n");
        kprint(" 17   11   DC1\n");
        kprint(" 18   12   DC2\n");
        kprint(" 19   13   DC3\n");
        kprint(" 20   14   DC4\n");
        kprint(" 21   15   NAK\n");
        kprint(" 22   16   SYN\n");
        kprint(" 23   17   ETB\n");
        kprint(" 24   18   CAN\n");
        kprint(" 25   19   EM\n");
        kprint(" 26   1A   SUB\n");
        kprint(" 27   1B   ESC\n");
        kprint(" 28   1C   FS\n");
        kprint(" 29   1D   GS\n");
        kprint(" 30   1E   RS\n");
        kprint(" 31   1F   US\n");

        kprint(" 32   20   SPACE\n");
        kprint(" 33   21   !\n");
        kprint(" 34   22   \"\n");
        kprint(" 35   23   #\n");
        kprint(" 36   24   $\n");
        kprint(" 37   25   %\n");
        kprint(" 38   26   &\n");
        kprint(" 39   27   '\n");
        kprint(" 40   28   (\n");
        kprint(" 41   29   )\n");
        kprint(" 42   2A   *\n");
        kprint(" 43   2B   +\n");
        kprint(" 44   2C   ,\n");
        kprint(" 45   2D   -\n");
        kprint(" 46   2E   .\n");
        kprint(" 47   2F   /\n");

        kprint(" 48   30   0\n");
        kprint(" 49   31   1\n");
        kprint(" 50   32   2\n");
        kprint(" 51   33   3\n");
        kprint(" 52   34   4\n");
        kprint(" 53   35   5\n");
        kprint(" 54   36   6\n");
        kprint(" 55   37   7\n");
        kprint(" 56   38   8\n");
        kprint(" 57   39   9\n");

        kprint(" 65   41   A\n");
        kprint(" 66   42   B\n");
        kprint(" 67   43   C\n");
        kprint(" 68   44   D\n");
        kprint(" 69   45   E\n");
        kprint(" 70   46   F\n");
        kprint(" 71   47   G\n");
        kprint(" 72   48   H\n");
        kprint(" 73   49   I\n");
        kprint(" 74   4A   J\n");
        kprint(" 75   4B   K\n");
        kprint(" 76   4C   L\n");
        kprint(" 77   4D   M\n");
        kprint(" 78   4E   N\n");
        kprint(" 79   4F   O\n");

        kprint(" 80   50   P\n");
        kprint(" 81   51   Q\n");
        kprint(" 82   52   R\n");
        kprint(" 83   53   S\n");
        kprint(" 84   54   T\n");
        kprint(" 85   55   U\n");
        kprint(" 86   56   V\n");
        kprint(" 87   57   W\n");
        kprint(" 88   58   X\n");
        kprint(" 89   59   Y\n");
        kprint(" 90   5A   Z\n");

        kprint(" 91   5B   [\n");
        kprint(" 92   5C   \\\n");
        kprint(" 93   5D   ]\n");
        kprint(" 94   5E   ^\n");
        kprint(" 95   5F   _\n");
        kprint(" 96   60   `\n");

        kprint(" 97   61   a\n");
        kprint(" 98   62   b\n");
        kprint(" 99   63   c\n");
        kprint("100   64   d\n");
        kprint("101   65   e\n");
        kprint("102   66   f\n");
        kprint("103   67   g\n");
        kprint("104   68   h\n");
        kprint("105   69   i\n");
        kprint("106   6A   j\n");
        kprint("107   6B   k\n");
        kprint("108   6C   l\n");
        kprint("109   6D   m\n");
        kprint("110   6E   n\n");
        kprint("111   6F   o\n");

        kprint("112   70   p\n");
        kprint("113   71   q\n");
        kprint("114   72   r\n");
        kprint("115   73   s\n");
        kprint("116   74   t\n");
        kprint("117   75   u\n");
        kprint("118   76   v\n");
        kprint("119   77   w\n");
        kprint("120   78   x\n");
        kprint("121   79   y\n");
        kprint("122   7A   z\n");

        kprint("123   7B   {\n");
        kprint("124   7C   |\n");
        kprint("125   7D   }\n");
        kprint("126   7E   ~\n");
        kprint("127   7F   DEL\n");
    }
    else
    {
        tty_write("Unknown command\n");
        k_log("Unknown Command has been run in CMD");
    }
}
