# CoreSys ABI Specification v1.1 (FÈUE)
## Core I ABI Spec FÈUE

## Overview

The CoreSys ABI (Application Binary Interface) defines the binary contract between:

* User Programs ↔ Kernel
* Kernel ↔ HAL
* Drivers ↔ Core Runtime

CoreSys uses a frame-based ABI rather than a traditional software interrupt interface.

---

# Architecture

**Target Architecture**

* x86_64
* Little Endian
* 64-bit pointers
* LP64 data model
* 64 bit system

When you follow the Core ABI use these types:

Type        | Underlying Type | Size (x86_64)
------------|----------------|----------------
ret_t       | int            | 32 bits
num_t       | int            | 32 bits
set_t       | const          | 64-bit pointer
let_t       | char           | 8 bits
nret_t      | void           | 0 bits
unum8_t     | uint8_t        | 8 bits
unum16_t    | uint16_t       | 16 bits
unum32_t    | uint32_t       | 32 bits
unum64_t    | uint64_t       | 64 bits
num8_t      | int8_t         | 8 bits
num16_t     | int16_t        | 16 bits
num32_t     | int32_t        | 32 bits
num64_t     | int64_t        | 64 bits
con_t       | signed int     | 32 bits
uncon_t     | unsigned int   | 32 bits
cap_t       | size_t         | 64 bits
vol_t       | volatile       | 0  bits
aut_t       | auto           | 0  bits
reg_t       | register       | 0  bits
str_t       | struct         | 0  bits
sta_t       | static         | 0  bits
unum128_t   | uint64_t       | 128 bits
num128_t    | int64_t        | 128 bits
tid_t       | char[16]       | 16 bytes

(If the type dont have a core abi varient use the normal type)


## Pointers

We use 
```c
num_t *x;
```
not
```c
num_t* x;
``` 

## Functions

We use 
```c
void ex() {

}
```
not
```c
void ex() 
{
    
}
``` 

Some drivers may use wrong because AI

# Syscall ABI

## Syscall Frame

```c
typedef struct {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
} syscall_frame_t;
```

## Register Usage

| Field | Purpose    |
| ----- | ---------- |
| rax   | Syscall ID |
| rdi   | Argument 1 |
| rsi   | Argument 2 |
| rdx   | Argument 3 |

Return value:

```c
return uint64_t;

rax
```

---

# Syscall Numbers

```c
enum syscalls {
    SYS_DEV_NULL = 0,
    SYS_READ = 1,
    SYS_WRITE = 2,
    SYS_SHUTDOWN = 3,
    SYS_REBOOT = 4,
    SYS_INIT = 5,
    SYS_DEINIT = 6,
    SYS_CLEAR = 7,
    SYS_REINIT = 8,
    SYS_HALT = 9,
    SYS_SF = 10
};
```

---

## SYS_DEV_NULL

### Number

```c
0
```

### Prototype

```c
int sys_dev_null(void);
```

### Description

Returns 0.

### Return

```c
0
```

---

## SYS_READ

### Number

```c
1
```

### Prototype

```c
char sys_read(void);
```

### Description

Reads one character from the serial device.

### Return

ASCII character.

---

## SYS_WRITE

### Number

```c
2
```

### Prototype

```c
void sys_write(const char* text);
```

### Arguments

| Register | Value             |
| -------- | ----------------- |
| rdi      | pointer to string |

### Description

Prints a NULL terminated string.

---

## SYS_SHUTDOWN

### Number

```c
3
```

### Description

Initiates ACPI shutdown.

---

## SYS_REBOOT

### Number

```c
4
```

### Description

Initiates system reboot.

---

## SYS_INIT

### Number

```c
5
```

### Description

Runs kernel initialization sequence.

---

## SYS_DEINIT

### Number

```c
6
```

### Description

Runs kernel deinitialization sequence.

---

## SYS_CLEAR

### Number

```c
7
```

### Description

Clears the active terminal.

---

## SYS_REINIT

### Number

```c
8
```

### Description

Performs:

```text
DEINIT
INIT
```

---

## SYS_HALT

### Number

```c
9
```

### Description

Disables interrupts and halts CPU.

Equivalent:

```asm
cli
hlt
```

---

## SYS_SF

### Number

```c
10
```

### Description

Triggers Kernel System Failure handler.

### Arguments

| Register | Value         |
| -------- | ------------- |
| rdi      | error message |

---

# HAL ABI

The HAL ABI provides low-level driver services.

---

## HAL Frame

```c
typedef struct {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
} hal_frame_t;
```

---

# HAL Call Numbers

```c
HAL_DEV_NULL          = 0
HAL_EXECUTE_COMMAND   = 1
HAL_TTY_LOOP          = 2
HAL_TTY_WRITE         = 3
HAL_TTY_PUTC          = 4
HAL_OUTB              = 5
HAL_INB               = 6
HAL_OUTW              = 7
HAL_INW               = 8
HAL_SERIAL_TX_EMPTY   = 9
HAL_SERIAL_RECEIVED   = 10
HAL_SERIAL_WRITE_CHAR = 11
HAL_A_CHAR_PRINT      = 12
HAL_SERIAL_WRITE      = 13
HAL_SERIAL_WRITE_U64  = 14
HAL_SERIAL_WRITE_PTR  = 15
HAL_KPRINT            = 16
HAL_KPRINT_CHAR       = 17
HAL_KPRINT_U64        = 18
HAL_KPRINT_U8         = 19
HAL_KPRINTF           = 20
HAL_SERIAL_READ_CHAR  = 21
HAL_KREAD             = 22
HAL_SERIAL_CLEAR      = 23
HAL_KCLEAR            = 24
HAL_K_CLEAR           = 25
HAL_KSTRLEN           = 26
HAL_KPRINT_STR        = 27
HAL_KPRINT_UINT       = 28
HAL_KPRINT_INT        = 29
HAL_INIT_SERIAL       = 30
HAL_DEINIT_SERIAL     = 31
HAL_LED_DEMO          = 32
HAL_SHUTDOWN          = 33
HAL_REBOOT            = 34
```

---

# FS ABI

The FS ABI provides low-level fs services.

---

## FS Frame

```c
typedef struct {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
} fs_frame_t;
```

---

# FS Call Numbers

```c
FS_DEV_NULL      = 0
COP_INIT         = 1
COP_DEINIT       = 2
COP_CREATE_DIR   = 3
COP_WRITE_FILE   = 4
COP_READ_FILE    = 5
COP_DELETE_FILE  = 6
COP_APPEND_FILE  = 7
COP_EXEC_FILE    = 8
COP_FS_INIT      = 9
COP_FS_DEINIT    = 10
COP_CREATE_FILE  = 11
```

---

# IRQ Call Numbers

00 - IRQ_DEV_NULL

100 - IRQ_DE   (#DE Divide Error)
101 - IRQ_DB   (#DB Debug)
102 - IRQ_NMI  (Non-Maskable Interrupt)
103 - IRQ_BP   (#BP Breakpoint)
104 - IRQ_OF   (#OF Overflow)
105 - IRQ_BR   (#BR BOUND Range Exceeded)
106 - IRQ_UD   (#UD Invalid Opcode)
107 - IRQ_NM   (#NM Device Not Available)
108 - IRQ_DF   (#DF Double Fault)
109 - IRQ_CO   (Coprocessor Segment Overrun)
10A - IRQ_TS   (#TS Invalid TSS)
10B - IRQ_NP   (#NP Segment Not Present)
10C - IRQ_SS   (#SS Stack Segment Fault)
10D - IRQ_GP   (#GP General Protection)
10E - IRQ_PF   (#PF Page Fault)
10F - IRQ_RS   (Reserved)
110 - IRQ_MF   (#MF x87 Floating Point)
111 - IRQ_AC   (#AC Alignment Check)
112 - IRQ_MC   (#MC Machine Check)
113 - IRQ_XM   (#XM SIMD Floating Point)
114 - IRQ_VE   (#VE Virtualization)
115 - IRQ_CP   (#CP Control Protection)

11C - IRQ_HV   (#HV Hypervisor Injection)
11D - IRQ_VC   (#VC VMM Communication)
11E - IRQ_SX   (#SX Security Exception)

---

# Task ABI

## Task Structure

```c
typedef struct cs_task {
    const char* name;
    const char* source_header;
    const char* entry_name;
    void (*entry)(struct cs_task* self);
} cs_task;
```

---

## Execution

```c
task_run(&task);
```

Equivalent:

```c
task.entry(&task);
```

when:

```c
tsk_ready == 1
```

---

# CFS ABI

## Node Types

```c
typedef enum {
    cfs_FILE,
    cfs_DIR
} cfs_type;
```

---

## Filesystem Object

```c
typedef struct cfs_node {
    char name[64];

    cfs_type type;

    uint64_t size;
    uint8_t* data;

    struct cfs_node* parent;

    struct cfs_node* children[16];

    uint32_t child_count;
} cfs_node;
```

---

## Initial Filesystem Layout

```text
/
└── sys
    ├── kernel
    │   └── kernel.cfg
    └── system
        └── debug.cfg
        └── boot
            └── bc.sctfi
```

---

# Serial ABI

## COM Ports

```c
COM1 = 0x3F8
A_PORT = 0x2E8
```

## Default Serial Format

```text
115200 baud
8 data bits
No parity
1 stop bit
```

or:

```text
115200 8N1
```

---

# ASM ABI

## Asm format

In CoreSys we use .s or .S files and no .asm files and we use gcc or nasm

## ASM syscall in the kernel

Use extern sys_* from drivers/sys/main.h in your .s file

# CoreSys ABI Stability

Version: 1.0

Compatibility Guarantees:

* Syscall IDs remain stable.
* HAL IDs remain stable.
* Frame layout remains stable.
* Task ABI remains stable.
* CFS structures may expand but existing fields remain binary compatible.

Breaking changes require:

```text
ABI Major Version Increment
```

# IDT ABI 00-1F

| Vector | Decimal | ABI Symbol        | Exception                               | Description                                     |
| ------ | ------: | ----------------- | --------------------------------------- | ----------------------------------------------- |
| 00     |       0 | `_cs_asm_irq_de`  | #DE Divide Error                        | Division by zero or invalid DIV/IDIV operation  |
| 01     |       1 | `_cs_asm_irq_db`  | #DB Debug Exception                     | Debug registers, breakpoints, single-step       |
| 02     |       2 | `_cs_asm_irq_nmi` | NMI                                     | Non-maskable interrupt                          |
| 03     |       3 | `_cs_asm_irq_bp`  | #BP Breakpoint                          | INT3 breakpoint instruction                     |
| 04     |       4 | `_cs_asm_irq_of`  | #OF Overflow                            | INTO overflow exception                         |
| 05     |       5 | `_cs_asm_irq_br`  | #BR Bound Range Exceeded                | BOUND instruction exception                     |
| 06     |       6 | `_cs_asm_irq_ud`  | #UD Invalid Opcode                      | Undefined or invalid instruction                |
| 07     |       7 | `_cs_asm_irq_nm`  | #NM Device Not Available                | FPU/MMX/SIMD unavailable                        |
| 08     |       8 | `_cs_asm_irq_df`  | #DF Double Fault                        | Fatal exception during exception handling       |
| 09     |       9 | `_cs_asm_irq_cs`  | #CS CoreSys Coprocessor Segment Overrun | CoreSys ABI name for legacy reserved CSO vector |
| 0A     |      10 | `_cs_asm_irq_ts`  | #TS Invalid TSS                         | Invalid Task State Segment                      |
| 0B     |      11 | `_cs_asm_irq_np`  | #NP Segment Not Present                 | Segment unavailable                             |
| 0C     |      12 | `_cs_asm_irq_ss`  | #SS Stack-Segment Fault                 | Stack segment violation                         |
| 0D     |      13 | `_cs_asm_irq_gp`  | #GP General Protection Fault            | General protection violation                    |
| 0E     |      14 | `_cs_asm_irq_pf`  | #PF Page Fault                          | Memory paging violation                         |
| 0F     |      15 | `_cs_asm_irq_rs`  | #RS Reserved                            | Reserved Intel vector                           |
| 10     |      16 | `_cs_asm_irq_mf`  | #MF x87 Floating-Point Error            | x87 floating point exception                    |
| 11     |      17 | `_cs_asm_irq_ac`  | #AC Alignment Check                     | Alignment violation                             |
| 12     |      18 | `_cs_asm_irq_mc`  | #MC Machine Check                       | Hardware machine failure                        |
| 13     |      19 | `_cs_asm_irq_xm`  | #XM SIMD Floating-Point Exception       | SSE/SIMD exception                              |
| 14     |      20 | `_cs_asm_irq_ve`  | #VE Virtualization Exception            | Virtualization exception                        |
| 15     |      21 | `_cs_asm_irq_cp`  | #CP Control Protection Exception        | CET control protection violation                |
| 16     |      22 | `_cs_asm_irq_16`  | Reserved                                | Reserved vector                                 |
| 17     |      23 | `_cs_asm_irq_17`  | Reserved                                | Reserved vector                                 |
| 18     |      24 | `_cs_asm_irq_18`  | Reserved                                | Reserved vector                                 |
| 19     |      25 | `_cs_asm_irq_19`  | Reserved                                | Reserved vector                                 |
| 1A     |      26 | `_cs_asm_irq_1A`  | Reserved                                | Reserved vector                                 |
| 1B     |      27 | `_cs_asm_irq_1B`  | Reserved                                | Reserved vector                                 |
| 1C     |      28 | `_cs_asm_irq_hv`  | #HV Hypervisor Injection Exception      | Hypervisor injected exception                   |
| 1D     |      29 | `_cs_asm_irq_vc`  | #VC VMM Communication Exception         | Virtual machine communication exception         |
| 1E     |      30 | `_cs_asm_irq_sx`  | #SX Security Exception                  | Security-related exception                      |
| 1F     |      31 | `_cs_asm_irq_1F`  | Reserved                                | Reserved vector                                 |
