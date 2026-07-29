#pragma once

// Task management system for CoreSys kernel drivers API

// If you are lazy set source_header to be NULL or empty string, but it's recommended to set it to the header file of the driver for better debugging and module origin tracking.

#include <mod/globe.h>
#include <kernel/mem.h>

#include <drivers/serial/main.h>
#include <drivers/sf/main.h>
#include <misc/rnd.h>
extern int tsk_ready;

// TID Task ID

int kprintf(const char *fmt, ...);
char e_digit(uint64_t x);
void *memcpy(void *dest, const void *src, size_t n);

// FIX 1: removed `const` from tid so memcpy can write into it without casting away const (ABF workaround removed)
typedef struct cs_task {
    const char* name;
    const char* source_header;
    const char* entry_name;
    tid_t tid;                                 // was: const tid_t tid
    void (*entry)(struct cs_task* self);
} cs_task;

// FIX 2: added `static inline` to all functions defined in the header to prevent multiple-definition linker errors
static inline void generate_tid(tid_t *tid) {
    for (int i = 0; i < 16; i++) {
        (*tid)[i] = e_digit(k_trnd());
    }
    (*tid)[16] = '\0';
}

static inline void print_tid(tid_t tid) {
    for (int i = 0; i < 16; i++) {
        kprintf("%c", tid[i]);
    }
}
void init(struct cs_task* self);
static inline void initSerial(struct cs_task* self);
static inline void serial_clear(struct cs_task* self);
void cfs_init(struct cs_task* self);
void ahci_init(struct cs_task* self);
void pci_init(struct cs_task* self);
void cop_init(struct cs_task* self);
void cop_deinit(struct cs_task* self);
bool cop_exists(const char *path);
bool cop_mkdir(const char *path);
bool cop_create(const char *path);
bool cop_write(const char *path, const void *buffer, uint64_t size);
bool cop_delete(const char *path);
bool cop_append(const char *path, const void *buffer, uint64_t size);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *s);

// FIX 2 (continued): task_run also gets `static inline` for the same reason
static inline void task_run(cs_task* task)
{
    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    if (tsk_ready == 0)
    {
        if (debug != 2) { kprintf("[TASK] scheduler not ready\n"); }
        return;
    }

    if (!task)
    {
        if (debug != 2) { kprintf("[TASK] NULL task\n"); }
        return;
    }

    if (!task->entry)
    {
        if (debug != 2) { kprintf("[TASK] NULL entry\n"); }
        return;
    }
    
    bool no_fs_tid =
        strcmp(task->entry_name, "init") == 0 ||
        strcmp(task->entry_name, "initSerial") == 0 ||
        strcmp(task->entry_name, "serial_clear") == 0 ||
        strcmp(task->entry_name, "cfs_init") == 0 ||
        strcmp(task->entry_name, "ahci_init") == 0 ||
        strcmp(task->entry_name, "pci_init") == 0 ||
        strcmp(task->entry_name, "cop_init") == 0 ||
        strcmp(task->entry_name, "cop_deinit") == 0;

    tid_t tid;
    generate_tid(&tid);

    // FIX 1 (continued): memcpy is now safe; no const cast needed
    memcpy((void*)task->tid, tid, sizeof(tid_t));

    bool tsk_cop = false;

    // This tasks will not get a fs based TID:
    // init, initSerial, serial_clear, cfs_init, ahci_init, pci_init, cop_init

    if (cop_g_inited == false || cop_g_mounted == false)
    {
        if (!no_fs_tid)
        {
            if (debug != 2)
            {
                kprintf("[WARN] [TASK] COP is not mounted or inited (Entry Point Name: %s)\n",
                        task->entry_name);
            }

            tsk_cop = true;
        }  // FIX 3: closing brace was misplaced — tsk_cop=true and its kprintf were in mismatched scopes

        if (debug == 1) {
            kprintf("[WARN] [TASK] '%s'-task (BOOT TASK) will not get a fs based TID (/task/[tid]) in COPFS\n", task->entry_name);
        }
    } else {
        if (cop_exists("/task/.cfsts") == true) {
            cop_delete("/task");
            cop_mkdir("/task");
        }
    }

    char path[128];

    if (!tsk_cop && !no_fs_tid) {
        // We are in a COPFS ready task env

        snprintf(path, sizeof(path), "/task/%s", task->tid);

        if (cop_exists(path) == true) {
            k_sff("[TASK] TID %s does alrady exist in /task/%s (Duble TID)", task->tid, task->tid);
        }

        if (cop_exists("/task") == false) {
            cop_mkdir("/task");
            if (debug == 1) {
                kprintf("Created /task\n");
            }
        } else {
            if (debug == 1) {
                kprintf("Exists /task\n");
            }
        }

        cop_create(path);

        char buf[8192]; // FIX 4: was 8092 (typo), corrected to 8192
        snprintf(buf, sizeof(buf),
            "Name: %s\n"
            "Source Header: %s\n"
            "Entry Name: %s\n"
            "TID: %s\n"
            "Entry Pointer: %p",
            task->name, task->source_header, task->entry_name, task->tid, task->entry
        );

        cop_write(path, buf, strlen(buf));
    }

    if (debug == 1) {
        kprintf("[TASK] A CS_TASK has been called with Entry Pointer: %p Entry Name: %s Source Header: %s CS_TASK name: %s TID: ",
            task->entry, task->entry_name, task->source_header, task->name);
        print_tid(tid);
        kprintf("\n");
    }

    task->entry(task);

    if (!tsk_cop && !no_fs_tid) {
        if (cop_exists(path) == true) {
            cop_delete(path);
        } else {
            if (task->entry != (void (*)(struct cs_task*))init &&
                task->entry != (void (*)(struct cs_task*))initSerial &&
                task->entry != (void (*)(struct cs_task*))serial_clear &&
                task->entry != (void (*)(struct cs_task*))cfs_init &&
                task->entry != (void (*)(struct cs_task*))ahci_init &&
                task->entry != (void (*)(struct cs_task*))pci_init &&
                task->entry != (void (*)(struct cs_task*))cop_deinit &&
                task->entry != (void (*)(struct cs_task*))cop_init)
            {
                if (debug == 1) {
                    kprintf("[WARN] [TASK] Task %s want to exit but /task/%s do not exist\n", task->entry_name, task->tid);
                }
            }
        }
    }
}

// .cfsts = CoreSys FileSystem Temp Strategy marker

// LATEX:
// For 656 samples, the expected random variation is roughly:

// \[
// \sigma = \sqrt{np(1-p)}
// \]

// For one digit:

// \[
// \sigma = \sqrt{656 \times 0.1 \times 0.9}
// \]

// \[
// \sigma \approx 7.7
// \]

// Percentage variation:

// \[
// \frac{7.7}{656} \times 100 \approx 1.17\%
// \]

// Statistics from 41 generated TIDs (656 digits total)
//
// | Digit | Percentage | Difference from 10.00% |
// | ----- | ---------: | ----------------------: |
// | 0     |    10.37%  | +0.37%                 |
// | 1     |     9.60%  | -0.40%                 |
// | 2     |    10.52%  | +0.52%                 |
// | 3     |    10.98%  | +0.98% (largest +)     |
// | 4     |    10.82%  | +0.82%                 |
// | 5     |     9.15%  | -0.85% (largest -)     |
// | 6     |     9.60%  | -0.40%                 |
// | 7     |     9.76%  | -0.24%                 |
// | 8     |     9.91%  | -0.09%                 |
// | 9     |     9.30%  | -0.70%                 |
//
// Maximum deviation from the expected 10.00% is approximately ±1%.
// This is within the expected statistical variation for a sample of
// 656 random digits and does not indicate bias.
