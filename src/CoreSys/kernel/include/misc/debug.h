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
#include <misc/avx.h>
#include <misc/cpu.h>

#include <drivers/hw/ihb/main.h>
#include <drivers/hw/ISAB/main.h>
#include <drivers/hw/usb/main.h>
#include <drivers/hw/ACPI/main.h>
#include <drivers/hw/vga/main.h>
#include <drivers/hw/eth/main.h>

#include <kernel/mem.h>

#ifdef KERNEL
ret_t kmain();

void krnl_stack(void) {
    uint64_t stack;

    asm volatile(
        "mov %%rsp, %0"
        : "=r"(stack)
    );

    kprintf("Kernel stack: %p\n", stack);
}
#endif // KERNEL

void print_debug(cs_task *self) {
    (void)self;

    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    // Prints what trace means
    k_log("TRACE = DEBUG/LOG/INTERNAL");

    // Prints and gets register dump
    _cs_asm_get_regs(&regs);
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

    core.fs.exec_file("/bin/it.bin");

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

    // Intel Host Brige Debug info
    ihb_general_print();

    // Print IHB PCI conf debug info
    ihb_pci_conf_print();

    // Print IHB Memory debug info
    ihb_memory_print();

    // Print UEFI conf debug info
    ihb_uefi_conf_print();

    // Dump PCI windows
    ihb_dump_pci_windows();

    // Dump Mem Controller
    ihb_dump_memory_controller();

    // Dump IHB conf
    ihb_dump_bridge_configuration(
        0,0,0
    );

    // Verifys firmare setup
    ihb_verify_firmware_setup();

    // Detects hw misconf
    ihb_detect_hardware_misconfiguration();

    // Prints full ISA Brige dump
    isab_dump_full();

    // Prints USB general debug info
    usb_general_print();
    
    // Print UHCI info
    usb_uhci_init();

    // Prints xHCI info
    xhci_init(0, 1, 2);

    // Print general ACPI info
    acpi_general_print();

    // Prints E1000 info
    e1000_dump();

    #ifdef KERNEL
    // Prints Kernel entry
    kprintf("Kernel Entry: %p\n", kmain);

    // Prints Kernel stack
    krnl_stack();
    #endif // KERNEL

    // AVX 512
    kprintf("Before AVX init: ");
    cpu_avx(); // Prints AVX state

    // Enable AVX
    avx_enable();

    // AVX 512
    kprintf("After AVX init: ");
    cpu_avx();

    // SSE 128
    detect_sse();

    // CPU ID
    cpu_id();

    kprintf("CoreSys Kernel Version: %s\n", g_core.version);
}
