#pragma once

#include <drivers/irqcalls/main.h>
#include <kernel/version.h>

void execute_command(const char *cmd, int debug);
void tty_putc(char c);
void tty_write(const char *s);
void tty_loop();
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
int serial_received(void);
void serial_write_char(char c);
void a_char_print(char c);
void serial_write(const char *s);
void serial_write_u64(uint64_t value);
void serial_write_ptr(const void *p);
void kprint(const char *s);
void kprint_char(char c);
void kprint_u64(uint64_t v);
void kprint_u8(uint8_t v);
int kprintf(const char *fmt, ...);
char serial_read_char(void);
char kread(void);
void kclear(void);
void k_clear(cs_task* self);
int kstrlen(const char *s);
void kprint_str(const char *s);
void kprint_uint(unsigned long long v, unsigned base);
void kprint_int(long long v);
void initSerial(cs_task* self);
void deinitSerial(cs_task* self);
void shutdown(void);
void reboot(void);

static inline void task_run(cs_task* task);

static inline int hal_dev_null(int code);
static inline void hal_execute_command(const char* cmd);

static inline void hal_tty_loop(void);
static inline void hal_tty_write(const char* s);
static inline void hal_tty_putc(char c);

static inline uint8_t hal_inb(uint16_t port);
static inline uint16_t hal_inw(uint16_t port);

static inline void hal_outb(uint16_t port, uint8_t val);
static inline void hal_outw(uint16_t port, uint16_t val);

static inline uint8_t hal_serial_tx_empty(void);
static inline uint8_t hal_serial_received(void);

static inline void hal_serial_write_char(char c);
static inline void hal_a_char_print(char c);
static inline void hal_serial_write(const char* s);

static inline void hal_serial_write_u64(uint64_t v);
static inline void hal_serial_write_ptr(const void* p);

static inline void hal_kprint(const char* s);
static inline void hal_kprint_char(char c);

static inline void hal_kprint_u64(uint64_t v);
static inline void hal_kprint_u8(uint8_t v);
static inline void hal_kprintf(const char* s);

static inline char hal_serial_read_char(void);
static inline char hal_kread(void);

static inline void hal_serial_clear(void);
static inline void hal_kclear(void);
static inline void hal_k_clear(void);

static inline size_t hal_kstrlen(const char* s);

static inline void hal_kprint_str(const char* s);

static inline void hal_kprint_uint(uint64_t v, unsigned base);
static inline void hal_kprint_int(int64_t v);

static inline void hal_init_serial(void);
static inline void hal_deinit_serial(void);

static inline void hal_led_demo(void);

static inline void hal_shutdown(void);
static inline void hal_reboot(void);

// System functions
int sys_dev_null(void);
char sys_read(void);
void sys_write(const char* s);
void sys_shutdown(void);
void sys_reboot(void);
void sys_init(void);
void sys_deinit(void);
void sys_clear(void);
void sys_reinit(void);
void sys_halt(void);
void sys_sf(const char* s);

// FS functions
int fsret(int code);
bool fs_cop_init();
uint64_t fs_cop_deinit();
int fs_cop_mkdir(const char *path);
int fs_cop_create(const char *path);
int fs_cop_write(const char *path, const void *buffer, uint64_t size);
int fs_cop_read(const char *path, void *buffer, uint64_t size);
int fs_cop_delete(const char *path);
int fs_cop_append(const char *path, const void *buffer, uint64_t size);
uint64_t fs_cop_exec_file(const char *path);
uint64_t fs_fs_init();
uint64_t fs_fs_deinit();

int memret(int code);
void pmm_init(uint64_t usable_start, uint64_t usable_size);
void pmm_deinit(cs_task *self);
void *pmm_alloc_pages(size_t pages);
void pmm_free_pages(void *ptr, size_t pages);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int val, size_t n);
uint64_t virt_to_phys(void *addr);

int tskret(int code);

typedef struct CS_FS {
    int (*null)(int code);

    bool (*init)(void);
    uint64_t (*deinit)(void);

    int (*mkdir)(const char *path);
    int (*create)(const char *path);

    int (*write)(const char *path, const void *buffer, uint64_t size);
    int (*read)(const char *path, void *buffer, uint64_t size);

    int (*delete)(const char *path);
    int (*append)(const char *path, const void *buffer, uint64_t size);

    uint64_t (*exec_file)(const char *path);

    uint64_t (*fs_init)(void);
    uint64_t (*fs_deinit)(void);
} __attribute__((packed)) CS_FS;

typedef struct CS_HAL {
    int (*null)(int);

    void (*execute_command)(const char*);
    void (*tty_loop)(void);
    void (*tty_write)(const char*);
    void (*tty_putc)(char);

    void (*outb)(uint16_t, uint8_t);
    uint8_t (*inb)(uint16_t);
    void (*outw)(uint16_t, uint16_t);
    uint16_t (*inw)(uint16_t);

    uint8_t (*serial_received)(void);
    void (*serial_write_char)(char);
    void (*a_char_print)(char);

    void (*serial_write)(const char*);
    void (*serial_write_u64)(uint64_t);
    void (*serial_write_ptr)(const void*);

    void (*kprint)(const char*);
    void (*kprint_char)(char);
    void (*kprint_u64)(uint64_t);
    void (*kprint_u8)(uint8_t);
    void (*kprintf)(const char*);

    char (*serial_read_char)(void);
    char (*kread)(void);

    void (*kclear)(void);
    void (*k_clear)(void);

    size_t (*kstrlen)(const char*);

    void (*kprint_str)(const char*);
    void (*kprint_uint)(unsigned long long, unsigned);
    void (*kprint_int)(long long);

    void (*init_serial)(void);
    void (*deinit_serial)(void);

    void (*shutdown)(void);
    void (*reboot)(void);
} __attribute__((packed)) CS_HAL;

typedef struct CS_SYS {
    int  (*null)(void);
    char (*read)(void);
    void (*write)(const char*);

    void (*shutdown)(void);
    void (*reboot)(void);

    void (*init)(void);
    void (*deinit)(void);
    void (*clear)(void);
    void (*reinit)(void);
    void (*halt)(void);

    void (*sf)(const char*);
} __attribute__((packed)) CS_SYS;

typedef struct CS_IRQ {
    uint64_t (*null)(int code);

    uint64_t (*de)(void);
    uint64_t (*db)(void);
    uint64_t (*nmi)(void);
    uint64_t (*bp)(void);
    uint64_t (*of)(void);
    uint64_t (*br)(void);
    uint64_t (*ud)(void);
    uint64_t (*nm)(void);
    uint64_t (*df)(void);
    uint64_t (*co)(void);
    uint64_t (*ts)(void);
    uint64_t (*np)(void);
    uint64_t (*ss)(void);
    uint64_t (*gp)(void);
    uint64_t (*pf)(void);
    uint64_t (*rs)(void);
    uint64_t (*mf)(void);
    uint64_t (*ac)(void);
    uint64_t (*mc)(void);
    uint64_t (*xm)(void);
    uint64_t (*ve)(void);
    uint64_t (*cp)(void);
    uint64_t (*hv)(void);
    uint64_t (*vc)(void);
    uint64_t (*sx)(void);

} __attribute__((packed)) CS_IRQ;

typedef struct CS__TASK {
    int  (*null)(int code);
    void (*task_run)(cs_task *task);
} __attribute__((packed)) CS__TASK;

typedef struct CS_MEM {
    int      (*null)(int code);
    void     (*pmm_init)(uint64_t usable_start, uint64_t usable_size);
    void     (*pmm_deinit)(cs_task *self);
    void    *(*pmm_alloc_pages)(size_t pages);
    void     (*pmm_free_pages)(void *ptr, size_t pages);
    void    *(*kmalloc)(size_t size);
    void     (*kfree)(void *ptr);
    void    *(*memcpy)(void *dest, const void *src, size_t n);
    void    *(*memset)(void *dest, int val, size_t n);
    uint64_t (*virt_to_phys)(void *addr);
} __attribute__((packed)) CS_MEM;

typedef struct CS_CORE {
    CS_FS  fs; // Fscalls
    CS_HAL hal; // Halcalls
    CS_SYS sys; // Syscalls
    CS_IRQ irq; // Irqcalls
    CS_MEM mem; // Memcalls
    CS__TASK task; // Taskcalls
    char version[16]; // Version
} __attribute__((packed)) CS_CORE;

void tsk_init(void) {
    tsk_ready = 1;
}

void tsk_deinit(void) {
    tsk_ready = 0;
}

void cs_init(CS_CORE *core)
{
    tsk_init();

    snprintf(core->version, sizeof((void*)CS_VER), "%s", CS_VER);

    core->task.task_run  = task_run;
    core->task.null      = tskret;

    core->mem.null = memret;

    core->mem.pmm_init        = pmm_init;
    core->mem.pmm_deinit      = pmm_deinit;
    core->mem.pmm_alloc_pages = pmm_alloc_pages;
    core->mem.pmm_free_pages  = pmm_free_pages;

    core->mem.kmalloc = kmalloc;
    core->mem.kfree   = kfree;

    core->mem.memcpy = memcpy;
    core->mem.memset = memset;

    core->mem.virt_to_phys = virt_to_phys;

    core->fs.null        = fsret;

    core->fs.init        = fs_cop_init;
    core->fs.deinit      = fs_cop_deinit;

    core->fs.mkdir       = fs_cop_mkdir;
    core->fs.create      = fs_cop_create;

    core->fs.write       = fs_cop_write;
    core->fs.read        = fs_cop_read;

    core->fs.delete      = fs_cop_delete;
    core->fs.append      = fs_cop_append;

    core->fs.exec_file   = fs_cop_exec_file;

    core->fs.fs_init     = fs_fs_init;
    core->fs.fs_deinit   = fs_fs_deinit;

    core->irq.null = irq_dev_null;

    core->irq.de  = irq_de;
    core->irq.db  = irq_db;
    core->irq.nmi = irq_nmi;
    core->irq.bp  = irq_bp;
    core->irq.of  = irq_of;
    core->irq.br  = irq_br;
    core->irq.ud  = irq_ud;
    core->irq.nm  = irq_nm;

    core->irq.df  = irq_df;
    core->irq.co  = irq_co;

    core->irq.ts  = irq_ts;
    core->irq.np  = irq_np;
    core->irq.ss  = irq_ss;
    core->irq.gp  = irq_gp;
    core->irq.pf  = irq_pf;

    core->irq.rs  = irq_rs;

    core->irq.mf  = irq_mf;
    core->irq.ac  = irq_ac;
    core->irq.mc  = irq_mc;
    core->irq.xm  = irq_xm;
    core->irq.ve  = irq_ve;
    core->irq.cp  = irq_cp;

    core->irq.hv  = irq_hv;
    core->irq.vc  = irq_vc;
    core->irq.sx  = irq_sx;

    core->hal.null              = hal_dev_null;

    core->hal.execute_command   = hal_execute_command;
    core->hal.tty_loop          = hal_tty_loop;
    core->hal.tty_write         = hal_tty_write;
    core->hal.tty_putc          = hal_tty_putc;

    core->hal.outb              = hal_outb;
    core->hal.inb               = hal_inb;
    core->hal.outw              = hal_outw;
    core->hal.inw               = hal_inw;

    core->hal.serial_received   = hal_serial_received;
    core->hal.serial_write_char = hal_serial_write_char;
    core->hal.a_char_print      = hal_a_char_print;

    core->hal.serial_write      = hal_serial_write;
    core->hal.serial_write_u64  = hal_serial_write_u64;
    core->hal.serial_write_ptr  = hal_serial_write_ptr;

    core->hal.kprint            = hal_kprint;
    core->hal.kprint_char       = hal_kprint_char;
    core->hal.kprint_u64        = hal_kprint_u64;
    core->hal.kprint_u8         = hal_kprint_u8;
    core->hal.kprintf           = hal_kprintf;

    core->hal.serial_read_char  = hal_serial_read_char;
    core->hal.kread             = hal_kread;

    core->hal.kclear            = hal_kclear;
    core->hal.k_clear           = hal_k_clear;

    core->hal.kstrlen           = hal_kstrlen;

    core->hal.kprint_str        = hal_kprint_str;
    core->hal.kprint_uint       = hal_kprint_uint;
    core->hal.kprint_int        = hal_kprint_int;

    core->hal.init_serial       = hal_init_serial;
    core->hal.deinit_serial     = hal_deinit_serial;

    core->hal.shutdown          = hal_shutdown;
    core->hal.reboot            = hal_reboot;

    core->sys.null      = sys_dev_null;
    core->sys.read      = sys_read;
    core->sys.write     = sys_write;

    core->sys.shutdown = sys_shutdown;
    core->sys.reboot   = sys_reboot;

    core->sys.init      = sys_init;
    core->sys.deinit    = sys_deinit;
    core->sys.clear     = sys_clear;
    core->sys.reinit    = sys_reinit;
    core->sys.halt      = sys_halt;
    core->sys.sf        = sys_sf;
}

void cs_deinit(CS_CORE *core)
{
    tsk_deinit();

    memcpy(core->version, 0, 0);

    core->task.task_run  = NULL;

    core->mem.pmm_init        = NULL;
    core->mem.pmm_deinit      = NULL;
    core->mem.pmm_alloc_pages = NULL;
    core->mem.pmm_free_pages  = NULL;

    core->mem.kmalloc = NULL;
    core->mem.kfree   = NULL;

    core->mem.memcpy = NULL;
    core->mem.memset = NULL;

    core->mem.virt_to_phys = NULL;

    core->irq.null = NULL;

    core->irq.de  = NULL;
    core->irq.db  = NULL;
    core->irq.nmi = NULL;
    core->irq.bp  = NULL;
    core->irq.of  = NULL;
    core->irq.br  = NULL;
    core->irq.ud  = NULL;
    core->irq.nm  = NULL;

    core->irq.df  = NULL;
    core->irq.co  = NULL;

    core->irq.ts  = NULL;
    core->irq.np  = NULL;
    core->irq.ss  = NULL;
    core->irq.gp  = NULL;
    core->irq.pf  = NULL;

    core->irq.rs  = NULL;

    core->irq.mf  = NULL;
    core->irq.ac  = NULL;
    core->irq.mc  = NULL;
    core->irq.xm  = NULL;
    core->irq.ve  = NULL;
    core->irq.cp  = NULL;

    core->irq.hv  = NULL;
    core->irq.vc  = NULL;
    core->irq.sx  = NULL;

    core->fs.null        = NULL;

    core->fs.init        = NULL;
    core->fs.deinit      = NULL;

    core->fs.mkdir       = NULL;
    core->fs.create      = NULL;

    core->fs.write       = NULL;
    core->fs.read        = NULL;

    core->fs.delete      = NULL;
    core->fs.append      = NULL;

    core->fs.exec_file   = NULL;

    core->fs.fs_init     = NULL;
    core->fs.fs_deinit   = NULL;

    core->hal.execute_command   = NULL;
    core->hal.tty_loop          = NULL;
    core->hal.tty_write         = NULL;
    core->hal.tty_putc          = NULL;

    core->hal.outb              = NULL;
    core->hal.inb               = NULL;
    core->hal.outw              = NULL;
    core->hal.inw               = NULL;

    core->hal.serial_received   = NULL;
    core->hal.serial_write_char = NULL;
    core->hal.a_char_print      = NULL;

    core->hal.serial_write      = NULL;
    core->hal.serial_write_u64  = NULL;
    core->hal.serial_write_ptr  = NULL;

    core->hal.kprint            = NULL;
    core->hal.kprint_char       = NULL;
    core->hal.kprint_u64        = NULL;
    core->hal.kprint_u8         = NULL;
    core->hal.kprintf           = NULL;

    core->hal.serial_read_char  = NULL;
    core->hal.kread             = NULL;

    core->hal.kclear            = NULL;
    core->hal.k_clear           = NULL;

    core->hal.kstrlen           = NULL;
    core->hal.kprint_str        = NULL;
    core->hal.kprint_uint       = NULL;
    core->hal.kprint_int        = NULL;

    core->hal.init_serial       = NULL;
    core->hal.deinit_serial     = NULL;

    core->hal.shutdown          = NULL;
    core->hal.reboot            = NULL;

    core->sys.read             = NULL;
    core->sys.write            = NULL;

    core->sys.shutdown         = NULL;
    core->sys.reboot           = NULL;

    core->sys.init              = NULL;
    core->sys.deinit            = NULL;
    core->sys.clear             = NULL;
    core->sys.reinit            = NULL;
    core->sys.halt              = NULL;
    core->sys.sf                = NULL;
}
