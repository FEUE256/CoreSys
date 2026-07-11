#pragma once

#include <mod/globe.h>
#include <mod/types.h>
#include <mod/assert.h>
#include <mod/status.h>

#include <asm/global.h>

#include <drivers/vga/main.h>

#include <drivers/ret/main.h>
#include <drivers/status/main.h>
#include <drivers/pci/main.h>
#include <drivers/ahci/main.h>
#include <drivers/irq/main.h>
#include <drivers/ata/main.h>

#include <drivers/ACPI/main.h>
#include <drivers/led/main.h>
#include <drivers/serial/main.h>
#include <drivers/time/main.h>

#include <drivers/tty/main.h>
#include <drivers/tty/cmd.h>

#include <drivers/halcalls/main.h>
#include <drivers/hal/main.h>

#include <drivers/fscalls/main.h>
#include <drivers/fs/main.h>
#include <drivers/fscalls/main.h>
#include <drivers/init/main.h>
#include <drivers/task/main.h>
#include <drivers/halt/main.h>
#include <drivers/sf/main.h>

#include <drivers/syscalls/main.h>
#include <drivers/sys/main.h>

#include <drivers/cfs/main.h>
#include <drivers/log/main.h>
#include <drivers/page/main.h>
#include <drivers/reg/main.h>

#include <drivers/nvme/main.h>

#include <drivers/cop/main.h>

#include <misc/pp.h>

#include <kernel/mem.h>

void print_debug(cs_task *self) {
    (void)self;

    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    // Prints what trace means
    k_log("TRACE = DEBUG/LOG/INTERNAL");

    // Prints register dump
    print_regs();

    // Prints the time at boot
    k_trace_n2("Time at Boot: ");
    get_time();

    // Prints the CMOS RegB
    k_trace_n2("CMOS RegB: ");
    get_regb();

    // Prints the debug int (it should be 1 if it prints)
    k_trace_n2("Debug Mode Int: ");
    kprint_int(debug);
    kprint("\n");

    // Prints the status and sstatus
    sprint();

    // Prints the PCI scan
    pci_scan();

    // Gets AHCI base
    unumarr_t ahci_base =
        ahci_get_base(0, 31, 2);

    // Prints the AHCI base
    kprintf("AHCI base: %x\n", ahci_base);

    // Prints the ahci ports
    ahci_lp();

    // Prints the iGPU/GPU Info
    pci_igpu_info();

    // Print NVMe scan
    nvme_scan(); 

    print_compiler_info(); // Prints Pre Processor __***__ macros

    compile(); // Prints when the kernel was compiled

    // Prints boot count
    char bc_buf[64];
    bc_print(bc_buf);

    kprintf("Boot count: %s\n", bc_buf);

    core.fs.exec_file("/bin/test.bin");

    // Tests pointers
    num_t x = 5;
    kprintf("Test addesses: %d @ %p\n", x, &x);

    // Test NULL pointers
    int *pN = NULL;
    kprintf("NULL Pointer %%p pN %p NULL Pointer %%d pN %d\n", pN, pN);
    kprintf("NULL Pointer %%p *pN %p NULL Pointer %%d *pN %d\n", *pN, *pN);
    kprintf("NULL Pointer %%p &pN %p NULL Pointer %%d &pN %d\n", &pN, &pN);

    // Test NULL
    kprintf("NULL %%p NULL %p NULL %%d NULL %d\n", NULL, NULL);

    // Prints debug texts
    k_warning("[TRACE] The kernel is only for QEMU and may not work properly on real hardware. If trying to use on real hardware can result in system damages. This OS is under active development, use at your own risk.");
    k_log("[TRACE] FÈUE CoreSys Kernel Terminal will be started in one secund. Subsystem status: Boot: OK, Init: OK, Kernel started: YES, Logging: ACTIVE, Debug Int: Available");
    k_log("[TRACE] Cheack 'sstatus' type CS_SUBSYS_STATUS_T for verdification and 'status' type CS_STATUS_T for general status");
}