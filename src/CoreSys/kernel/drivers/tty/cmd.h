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
#include <drivers/nvme/main.h>
#include <drivers/cop/main.h>
#include <drivers/hw/ACPI/main.h>
#include <misc/debug.h>
#include <misc/rnd.h>
#include <drivers/tty/state.h>
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

void execute_command(const char *cmd, int debug)
{
    (void)debug;
    // HELP
    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p')
    {
    tty_write(
        "CoreSys Command List\n"
        "--------------------\n"
        " ata       - Dump ATA sector\n"
        " cfg       - Show kernel configuration\n"
        " clear     - Clear terminal\n"
        " cls       - Alias for clear\n"
        " cr        - Show credits\n"
        " debug     - Show debug value\n"
        " echo      - Print text\n"
        " fsf       - Formats the FS"
        " fsinfo    - Show filesystem information\n"
        " help      - Show this help\n"
        " hlt       - Halt CPU\n"
        " ii        - Show ASCII table\n"
        " led       - Keyboard LED demo\n"
        " nvme      - Dump NVMe sector\n"
        " ps        - List running processes\n"
        " rax       - Print RAX register\n"
        " reboot    - Reboot system\n"
        " reg       - Dump CPU registers (debug mode)\n"
        " sf        - Trigger system failure test\n"
        " s0..s5    - Enter ACPI sleep state S0-S5\n"
        " shutdown  - Shut down system\n"
        " ver       - Show kernel version\n"
        " mon       - Shell monitring\n"
        " pwd       - Show the current dir\n"
        " cd        - Change dir\n"
        " ls        - List files and dirs\n"
        " mkdir     - Creates a dir\n"
        " touch     - Creates a file\n"
        " exist     - Prints if the dir of file exists\n"
        " tree      - Prints the fs tree\n"
    );
    }
    // PWD - show current directory
    else if (cmd[0] == 'p' && cmd[1] == 'w' && cmd[2] == 'd' && cmd[3] == '\0')
    {
        tty_write(tty_terms[tty_current].cwd);
        tty_write("\r\n");
    }
    // CLEAR / CLS
    else if (
        (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r') ||
        (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 's')
    )
    {
        tty_write("\x1b[2J\x1b[H");
    }
    else if (
        cmd[0] == 's' &&
        cmd[1] >= '0' && cmd[1] <= '5' &&
        cmd[2] == '\0'
    )
    {
        uint8_t state = (uint8_t)(cmd[1] - '0');

        acpi_sleep(state);
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
    else if (cmd[0] == 'r' && cmd[1] == 'n' && cmd[2] == 'd')
    {
        uint64_t value = k_trnd();

        kprintf("Random Numbers: ");
        kprint_u64(value);
        kprint("\n");
    }
    else if (cmd[0] == 's' && cmd[1] == 'h' && cmd[2] == 'u' && cmd[3] == 't' && cmd[4] == 'd' && cmd[5] == 'o' && cmd[6] == 'w' && cmd[7] == 'n') {
        shutdown();
    }
    else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'b' && cmd[3] == 'o' && cmd[4] == 'o' && cmd[5] == 't') {
        reboot();
    }
    else if (cmd[0] == 'h' && cmd[1] == 'l' && cmd[2] == 't')
    {
        cs_task hlt_task = {
            .name = "Halt Task",
            .source_header = "drivers/halt/main.h",
            .entry_name = "hlt",
            .entry = hlt
        };
        task_run(&hlt_task); // Halt the system
    }
    // "mon" command - loops printing stats on current TTY
    else if (cmd[0] == 'm' && cmd[1] == 'o' && cmd[2] == 'n') {
        while (1) {
            char bc[64];
            cop_read("/sys/system/boot/bc.sctfi", bc, sizeof(bc));
            kprintf("\r[MON] Boot count: %s  Time: ", bc);
            get_time();

            // check for _ to break
            if (serial_received() && inb(0x3F8) == '_')
                break;
        }
    }

    // CD - change directory
    else if (cmd[0] == 'c' && cmd[1] == 'd')
    {
        const char *path = cmd + 2;
        if (*path == ' ') path++;

        // cd with no arg goes to /
        if (*path == '\0') {
            strcpy(tty_terms[tty_current].cwd, "/");
        }
        else if (path[0] == '/') {
            // absolute path
            if (cop_exists(path)) {
                strcpy(tty_terms[tty_current].cwd, path);
            } else {
                kprintf("cd: %s: No such directory\r\n", path);
            }
        }
        else if (strcmp(path, "..") == 0) {
            // strip last component
            if (!(tty_terms[tty_current].cwd[0] == '/' && tty_terms[tty_current].cwd[1] == '\0')) {
                char *last = strrchr(tty_terms[tty_current].cwd, '/');
                if (last && last != tty_terms[tty_current].cwd)
                    *last = '\0';
                else
                    tty_terms[tty_current].cwd[1] = '\0'; // back to "/"
            }
        }
        else {
            // relative path - build full path
            char full[256];
            if (tty_terms[tty_current].cwd[0] == '/' && tty_terms[tty_current].cwd[1] == '\0') {
                snprintf(full, sizeof(full), "/%s", path);
            } else {
                snprintf(full, sizeof(full), "%s/%s", tty_terms[tty_current].cwd, path);
            }

            if (cop_exists(full)) {
                strcpy(tty_terms[tty_current].cwd, full);
            } else {
                kprintf("cd: %s: No such directory\r\n", full);
            }
        }
    }

    // CAT - print file contents
    else if (cmd[0] == 'c' && cmd[1] == 'a' && cmd[2] == 't')
    {
        const char *path = cmd + 3;
        if (*path == ' ') path++;

        if (*path == '\0') {
            tty_write("cat: missing path\r\n");
        }
        else {
            // build full path if relative
            char full[256];
            if (path[0] == '/') {
                strcpy(full, path);
            } else if (tty_terms[tty_current].cwd[0] == '/' && tty_terms[tty_current].cwd[1] == '\0') {
                snprintf(full, sizeof(full), "/%s", path);
            } else {
                snprintf(full, sizeof(full), "%s/%s", tty_terms[tty_current].cwd, path);
            }

            if (!cop_exists(full)) {
                kprintf("cat: %s: No such file\r\n", full);
            } else {
                char buf[4096];
                if (cop_read(full, buf, sizeof(buf))) {
                    tty_write(buf);
                    tty_write("\r\n");
                } else {
                    kprintf("cat: %s: Read failed\r\n", full);
                }
            }
        }
    }

    // LS - list directory contents
    else if (cmd[0] == 'l' && cmd[1] == 's')
    {
        const char *path = cmd + 2;
        if (*path == ' ') path++;

        char *cwd = tty_terms[tty_current].cwd;

        char full[256];
        if (*path == '\0') {
            strcpy(full, cwd);
        } else if (path[0] == '/') {
            strcpy(full, path);
        } else if (cwd[0] == '/' && cwd[1] == '\0') {
            snprintf(full, sizeof(full), "/%s", path);
        } else {
            snprintf(full, sizeof(full), "%s/%s", cwd, path);
        }

        if (!cop_exists(full)) {
            kprintf("ls: %s: No such directory\r\n", full);
        } else {
            uint64_t inode_num;
            if (!cop_lookup(full, &inode_num)) {
                kprintf("ls: %s: lookup failed\r\n", full);
            } else {
                cop_inode_t inode;
                cop_read_inode(inode_num, &inode);

                if (inode.type != 2) {
                    kprintf("ls: %s: Not a directory\r\n", full);
                } else {
                    // total entries written = inode.size / sizeof(cop_dirent_t)
                    uint64_t total_entries = inode.size / sizeof(cop_dirent_t);
                    uint64_t entries_seen  = 0;

                    for (int i = 0; i < COP_DIRECT_BLOCKS && entries_seen < total_entries; i++) {
                        if (inode.blocks[i] == 0) continue;

                        // skip blocks outside data region
                        if (inode.blocks[i] < cop_g_sb.data_start) continue;
                        if (inode.blocks[i] >= cop_g_sb.total_blocks) continue;

                        uint8_t buf[COP_BLOCK_SIZE];
                        ata_read_blocks(
                            cop_block_to_lba(inode.blocks[i]),
                            buf,
                            COP_BLOCK_SIZE / COP_SECTOR_SIZE
                        );

                        uint64_t offset = 0;
                        while (offset + sizeof(cop_dirent_t) <= COP_BLOCK_SIZE
                            && entries_seen < total_entries)
                        {
                            cop_dirent_t *entry = (cop_dirent_t *)(buf + offset);

                            if (entry->inode != 0 && entry->name[0] != '\0') {
                                if (entry->type == 2)
                                    kprintf("[DIR]  %s\r\n", entry->name);
                                else
                                    kprintf("[FILE] %s\r\n", entry->name);
                            }

                            entries_seen++;
                            offset += sizeof(cop_dirent_t);
                        }
                    }
                }
            }
        }
    }

    else if (cmd[0] == 't' &&
            cmd[1] == 'r' &&
            cmd[2] == 'e' &&
            cmd[3] == 'e')
    {
        const char *path = cmd + 4;
        if (*path == ' ')
            path++;

        char *cwd = tty_terms[tty_current].cwd;

        char full[256];

        if (*path == '\0')
        {
            strcpy(full, cwd);
        }
        else if (path[0] == '/')
        {
            strcpy(full, path);
        }
        else if (cwd[0] == '/' && cwd[1] == '\0')
        {
            snprintf(full, sizeof(full), "/%s", path);
        }
        else
        {
            snprintf(full, sizeof(full), "%s/%s", cwd, path);
        }

        if (!cop_exists(full))
        {
            kprintf("tree: %s: No such directory\r\n", full);
        }
        else
        {
            tree_visited_count = 0;

            kprintf("%s\r\n", full);
            tree_print(full, 0);
        }
    }
    // MKDIR - create directory
    else if (cmd[0] == 'm' && cmd[1] == 'k' && cmd[2] == 'd' && cmd[3] == 'i' && cmd[4] == 'r')
    {
        const char *path = cmd + 5;
        if (*path == ' ') path++;

        char full[256];
        if (path[0] == '/') {
            strcpy(full, path);
        } else if (tty_terms[tty_current].cwd[0] == '/' && tty_terms[tty_current].cwd[1] == '\0') {
            snprintf(full, sizeof(full), "/%s", path);
        } else {
            snprintf(full, sizeof(full), "%s/%s", tty_terms[tty_current].cwd, path);
        }

        if (cop_mkdir(full))
            kprintf("mkdir: created %s\r\n", full);
        else
            kprintf("mkdir: failed to create %s\r\n", full);
    }

    // TOUCH - create empty file
    else if (cmd[0] == 't' && cmd[1] == 'o' && cmd[2] == 'u' && cmd[3] == 'c' && cmd[4] == 'h')
    {
        const char *path = cmd + 5;
        if (*path == ' ') path++;

        char full[256];
        if (path[0] == '/') {
            strcpy(full, path);
        } else if (tty_terms[tty_current].cwd[0] == '/' && tty_terms[tty_current].cwd[1] == '\0') {
            snprintf(full, sizeof(full), "/%s", path);
        } else {
            snprintf(full, sizeof(full), "%s/%s", tty_terms[tty_current].cwd, path);
        }

        if (cop_create(full))
            kprintf("touch: created %s\r\n", full);
        else
            kprintf("touch: failed to create %s\r\n", full);
    }

    // RM - delete file or directory
    else if (cmd[0] == 'r' && cmd[1] == 'm')
    {
        const char *path = cmd + 2;
        if (*path == ' ') path++;

        char full[256];
        if (path[0] == '/') {
            strcpy(full, path);
        } else if (tty_terms[tty_current].cwd[0] == '/' && tty_terms[tty_current].cwd[1] == '\0') {
            snprintf(full, sizeof(full), "/%s", path);
        } else {
            snprintf(full, sizeof(full), "%s/%s", tty_terms[tty_current].cwd, path);
        }

        if (!cop_exists(full)) {
            kprintf("rm: %s: No such file\r\n", full);
        } else if (cop_delete(full)) {
            kprintf("rm: deleted %s\r\n", full);
        } else {
            kprintf("rm: failed to delete %s\r\n", full);
        }
    }

    // EXISTS - check if path exists
    else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 's' && cmd[4] == 't')
    {
        const char *path = cmd + 5;
        // handle both "exist /foo" and "exists /foo"
        if (*path == 's') path++;
        if (*path == ' ') path++;

        char full[256];
        if (path[0] == '/') {
            strcpy(full, path);
        } else if (tty_terms[tty_current].cwd[0] == '/' && tty_terms[tty_current].cwd[1] == '\0') {
            snprintf(full, sizeof(full), "/%s", path);
        } else {
            snprintf(full, sizeof(full), "%s/%s", tty_terms[tty_current].cwd, path);
        }

        if (cop_exists(full))
            kprintf("exists: YES %s\r\n", full);
        else
            kprintf("exists: NO  %s\r\n", full);
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
        kprint("Filesystem: cfs and cop\n");
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
    else if (cmd[0] == 'f' && cmd[1] == 's' && cmd[2] == 'f')
    {
        cop_format(COP_TOTAL_LBAS);

        cs_task init_cop_task = {
            .name = "COP initialization Task",
            .source_header = "drivers/cop/main.h",
            .entry_name = "cop_init",
            .entry = cop_init
        };
        task_run(&init_cop_task); // init COP

        cs_task init_fs_task = {
            .name = "FS initialization Task",
            .source_header = "drivers/cop/main.h",
            .entry_name = "fs_init",
            .entry = fs_init
        };
        task_run(&init_fs_task); // init FS
        if (debug != 2) { k_log("FS initialized successfully."); }

        char buf[4092];
        cop_read("/sys/system/debug.cfg", buf, sizeof(buf));

        int bc = (uint64_t)kstrtoull(buf, NULL, 10);
        if (bc != 2) { k_log("COP initialized successfully."); }
    }
    else if (cmd[0] == 'c' && cmd[1] == 'f' && cmd[2] == 'g')
    {
        char buf[4092];
        cop_read("/sys/kernel/kernel.cfg", buf, sizeof(buf));

        kprintf("Kernel Config: \n");
        kprintf("%s", buf);

    }
    else if (cmd[0] == 'c' && cmd[1] == 'r')
    {
        kprint("Thanks to Queso Fuego (parts of code), elevatorguy (parts of code), Cyber::Boot (fork), Terry A. Davis (TempleOS), Ankit Kumar (Polaris OS), Neptune650 (Polaris OS), MishaTy (Polaris OS), redmine4404 (Polaris OS), AnalogFeelings (Polaris), 1010101001010101 (tinycrypt), chris-morrison (tinycrypt), mczraf (tinycrypt), Ipereira (tinycrypt), malsbat (tinycrypt), rob-brown (tinycrypt), haukepetersen (tinycrypt), mped-oticon (tinycrypt), thoh-ot (tinycrypt), daor-oti (tinycrypt), winnietwo (tinycrypt), sfblackl-intel (tinycrypt), every person on the EDK II team and many more for the inspiration and help in making this project possible! RIP Terry A. Davis!\r\n");
    }
    else if (cmd[0] == 'l' && cmd[1] == 'e' && cmd[2] == 'd') {
        led_demo();
    }
    else if (cmd[0] == 'a' && cmd[1] == 't' && cmd[2] == 'a') {
        ata_dump_lba_io(0);
    }
    else if (cmd[0] == 'd' && cmd[1] == 'e' && cmd[2] == 'b' && cmd[3] == 'u' && cmd[4] == 'g') {
        char buf[512];
        cop_read("/sys/system/debug.cfg", buf, sizeof(buf));

        kprintf("%s\n", buf);
    }
    else if (cmd[0] == 'n' && cmd[1] == 'v' && cmd[2] == 'm' && cmd[3] == 'e') {
        nvme_dump_drive_lba(0, 1, 0);
    }
    else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'g') {
        char buf[4092];
        cop_read("/sys/system/debug.cfg", buf, sizeof(buf));

        int bc = (uint64_t)kstrtoull(buf, NULL, 10);
    
        if (bc == 1) {
            _cs_asm_get_regs(&regs); // ASM / Regs global
            print_regs();
        } else {
            k_warning("Enable Debug mode for this feature\n");
        }
    }
    else if (cmd[0] == 'p' && cmd[1] == 'd') {
        char buf[4092];
        cop_read("/sys/system/debug.cfg", buf, sizeof(buf));

        int bc = (uint64_t)kstrtoull(buf, NULL, 10);
    
        if (bc == 1) {
            cs_task print_debug_task = {
                .name = "Print Debug Task",
                .source_header = "misc/debug.h",
                .entry_name = "print_debug",
                .entry = print_debug
            };
            
            task_run(&print_debug_task);

            // Sets the status that the debug has been OK
            status |= CS_DEBUG_OK;
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
