# CoreSys ABI Specification v1.0 (FÈUE)
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

| Type     | Size    |
| -------- | ------- |
| char     | 8 bits  |
| uint8_t  | 8 bits  |
| uint16_t | 16 bits |
| uint32_t | 32 bits |
| uint64_t | 64 bits |
| pointer  | 64 bits |

---

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

# Error Handling

Unknown syscall:

```asm
.word 0xFFFF
```

Unknown HAL call:

```asm
.word 0xFFFF
```

This intentionally generates:

```text
#UD
Invalid Opcode Exception
```

---

# Task ABI

## Task Structure

```c
typedef struct cs_task {
    const char* name;
    const char* source_header;
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
