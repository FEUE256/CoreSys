#pragma once

#include <drivers/pci/main.h>

/*
    GET AHCI MMIO BASE
*/

static inline uintptr_t ahci_get_base(uint8_t bus,
                                      uint8_t dev,
                                      uint8_t fn)
{
    uint32_t bar5 = pci_get_bar5(bus, dev, fn);

    if (bar5 & 0x1)
    {
        /*
            I/O space BAR (NOT used for AHCI normally)
        */
        return 0;
    }

    return (uintptr_t)(bar5 & ~0xFu);
}


typedef struct
{
    volatile uint32_t cap;      // 0x00 Controller capabilities
    volatile uint32_t ghc;      // 0x04 Global host control
    volatile uint32_t is;       // 0x08 Interrupt status
    volatile uint32_t pi;       // 0x0C Ports implemented
    volatile uint32_t vs;       // 0x10 Version
    volatile uint32_t ccc_ctl;  // 0x14 Command completion coalescing control
    volatile uint32_t ccc_pts;  // 0x18 Command completion coalescing ports
    volatile uint32_t em_loc;   // 0x1C Enclosure management location
    volatile uint32_t em_ctl;   // 0x20 Enclosure management control
    volatile uint32_t cap2;     // 0x24 Extended capabilities
    volatile uint32_t bohc;     // 0x28 BIOS/OS handoff control

    uint8_t  rsv0[0xA0 - 0x2C]; // reserved space up to port region start

} hba_mem_t;

/*
    PORT STRUCTURE (each port starts at 0x100 + 0x80 * n)
*/

typedef struct
{
    volatile uint32_t clb;      // 0x00 Command list base address
    volatile uint32_t clbu;     // 0x04 Command list base upper
    volatile uint32_t fb;       // 0x08 FIS base address
    volatile uint32_t fbu;      // 0x0C FIS base upper
    volatile uint32_t is;       // 0x10 Interrupt status
    volatile uint32_t ie;       // 0x14 Interrupt enable
    volatile uint32_t cmd;      // 0x18 Command and status
    volatile uint32_t rsv0;     // 0x1C Reserved
    volatile uint32_t tfd;      // 0x20 Task file data
    volatile uint32_t sig;      // 0x24 Signature
    volatile uint32_t ssts;     // 0x28 SATA status
    volatile uint32_t sctl;     // 0x2C SATA control
    volatile uint32_t serr;     // 0x30 SATA error
    volatile uint32_t sact;     // 0x34 SATA active
    volatile uint32_t ci;       // 0x38 Command issue
    volatile uint32_t sntf;     // 0x3C SATA notification

    uint8_t rsv1[0x80 - 0x40];  // pad to 0x80 bytes per port

} hba_port_t;

hba_mem_t* get_hba(uint8_t bus, uint8_t dev, uint8_t fn)
{
    uintptr_t base = ahci_get_base(bus, dev, fn);

    if (base == 0)
        return (hba_mem_t*)0;

    return (hba_mem_t*)base;
}

void ahci_probe(uint8_t bus, uint8_t dev, uint8_t fn)
{
    hba_mem_t* hba = get_hba(bus, dev, fn);

    if (!hba)
    {
        kprintf("No AHCI controller\n");
        return;
    }

    uint32_t cap = hba->cap;
    uint32_t pi  = hba->pi;
    uint32_t vs  = hba->vs;

    kprintf("AHCI CAP=%x PI=%x VS=%x\n", cap, pi, vs);
}

void ahci_list_ports(uintptr_t ahci_base)
{
    hba_mem_t* hba = (hba_mem_t*)ahci_base;

    uint32_t ports = hba->pi;

    for (int i = 0; i < 32; i++)
    {
        if (ports & (1U << i))
        {
            hba_port_t* port =
                (hba_port_t*)((uint8_t*)hba + 0x100 + (i * 0x80));

            kprintf(
                "AHCI Port %d signature=%x\n",
                i,
                port->sig
            );
        }
    }
}

void ahci_lp() {
    ahci_list_ports(ahci_get_base(0, 31, 2));
}


#define ATA_IDENTIFY 0xEC

typedef struct
{
    uint8_t  cfis[64];
    uint8_t  acmd[16];
    uint8_t  reserved[48];
    uint8_t  prdt_entry[16]; // simplified placeholder (real impl uses PRDT array)
} hba_cmd_header_t;

extern void *memset(void *dest, int val, size_t n);

static inline void ahci_wait_busy(volatile uint32_t* tfd)
{
    while (tfd[0] & (1 << 7)) {} // BSY
    while (tfd[0] & (1 << 3)) {} // DRQ
}

/*
    MAIN IDENTIFY FUNCTION
*/

int ahci_identify(hba_port_t* port)
{
    if (!port)
        return -1;

    port->cmd &= ~0x1;
    port->cmd &= ~(1 << 4);
    port->cmd &= ~(1 << 15);

    while (port->cmd & (1 << 15));

    hba_cmd_header_t* cmd =
        (hba_cmd_header_t*)kmalloc(sizeof(hba_cmd_header_t));

    uint8_t* identify_buf =
        (uint8_t*)kmalloc(512);

    if (!cmd || !identify_buf)
        return -2;

    memset(cmd, 0, sizeof(hba_cmd_header_t));
    memset(identify_buf, 0, 512);

    cmd->cfis[0] = 0x27;
    cmd->cfis[1] = 1 << 7;
    cmd->cfis[2] = 0xEC;

    uint64_t addr = (uint64_t)identify_buf;

    ((uint32_t*)cmd->prdt_entry)[0] = (uint32_t)addr;
    ((uint32_t*)cmd->prdt_entry)[1] = 0;
    ((uint32_t*)cmd->prdt_entry)[2] = 511;
    ((uint32_t*)cmd->prdt_entry)[3] = 0;

    port->ci = 1;

    while (port->ci & 1);

    if (port->tfd & 1)
        return -3;

    return 0;
}


#define ATA_CMD_READ_DMA_EXT 0x25

static int ahci_wait(hba_port_t* port)
{
    // Wait until BSY and DRQ clear
    while (port->tfd & (1 << 7)) {} // BSY
    while (port->tfd & (1 << 3)) {} // DRQ
    return 0;
}

int ahci_read_sector(hba_port_t* port, uint64_t lba, void* buf)
{
    if (!port || !buf)
        return -1;

    ahci_wait(port);

    /* 1. Stop command engine */
    port->cmd &= ~1;
    port->cmd &= ~(1 << 4);
    port->cmd &= ~(1 << 15);
    while (port->cmd & (1 << 15));

    /* 2. Setup command FIS */
    uint8_t fis[64];
    memset(fis, 0, sizeof(fis));

    fis[0] = 0x27;              // Host to device FIS
    fis[1] = 1 << 7;           // Command
    fis[2] = ATA_CMD_READ_DMA_EXT;

    fis[4] = (uint8_t)lba;
    fis[5] = (uint8_t)(lba >> 8);
    fis[6] = (uint8_t)(lba >> 16);
    fis[7] = 0;                // device register (LBA mode)

    fis[8]  = (uint8_t)(lba >> 24);
    fis[9]  = (uint8_t)(lba >> 32);
    fis[10] = (uint8_t)(lba >> 40);

    fis[12] = 1;               // sector count = 1

    /* 3. Command list entry */
    hba_cmd_header_t* cmd =
        (hba_cmd_header_t*)kmalloc(sizeof(hba_cmd_header_t));

    memset(cmd, 0, sizeof(hba_cmd_header_t));

    memcpy(cmd->cfis, fis, 64);

    /* 4. PRDT (1 entry) */
    uint64_t addr = (uint64_t)buf;

    ((uint32_t*)cmd->prdt_entry)[0] = (uint32_t)addr;
    ((uint32_t*)cmd->prdt_entry)[1] = 0;
    ((uint32_t*)cmd->prdt_entry)[2] = 511; // 512 bytes
    ((uint32_t*)cmd->prdt_entry)[3] = 0;

    /* 5. Issue command */
    port->ci = 1;

    /* 6. Wait completion */
    while (port->ci & 1);

    /* 7. Check errors */
    if (port->tfd & 1)
        return -2;

    return 0;
}

#define HBA_PxCMD_ST  (1)
#define HBA_PxCMD_FRE (1 << 4)
#define HBA_PxCMD_FR  (1 << 14)
#define HBA_PxCMD_CR  (1 << 15)

void ahci_port_start(hba_port_t* port)
{
    // stop command engine
    port->cmd &= ~HBA_PxCMD_ST;
    port->cmd &= ~HBA_PxCMD_FRE;

    // wait until stopped
    while (port->cmd & (HBA_PxCMD_CR | HBA_PxCMD_FR));

    uintptr_t clb = ((uintptr_t)kmalloc(1024 + 1023) + 1023) & ~(uintptr_t)1023;
    uintptr_t fb  = ((uintptr_t)kmalloc(256 + 255) + 255) & ~(uintptr_t)255;

    memset((void*)clb, 0, 1024);
    memset((void*)fb, 0, 256);

    port->clb = (uint32_t)clb;
    port->clbu = 0;

    port->fb = (uint32_t)fb;
    port->fbu = 0;

    // clear errors
    port->serr = 0xFFFFFFFF;

    // enable FIS receive + start
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

void ahci_init_ports(uintptr_t base)
{
    hba_mem_t* hba = (hba_mem_t*)base;

    uint32_t ports = hba->pi;

    for (int i = 0; i < 32; i++)
    {
        if (ports & (1U << i))
        {
            hba_port_t* port =
                (hba_port_t*)((uint8_t*)hba + 0x100 + (i * 0x80));

            kprintf("Init AHCI port %d\n", i);

            ahci_port_start(port);
        }
    }
}

void ahci_init(cs_task *self) {
    (void)self;

    uintptr_t ahci_base =
        ahci_get_base(0, 31, 2);
    ahci_init_ports(ahci_base);
}

void ahci_port_stop(hba_port_t* port)
{
    if (!port)
        return;

    /* Clear ST (start) */
    port->cmd &= ~HBA_PxCMD_ST;

    /* Clear FRE (FIS receive enable) */
    port->cmd &= ~HBA_PxCMD_FRE;

    /* Wait until both engine bits clear */
    while (port->cmd & (HBA_PxCMD_CR | HBA_PxCMD_FR));
}

typedef struct hba_cmd_sheader
{
    uint16_t flags;      // command flags
    uint16_t prdtl;      // PRDT entries count
    uint32_t prdbc;      // PRD byte count

    uint64_t ctba;       // command table base address
    uint64_t ctbau;      // upper 64-bit (if needed)

    uint32_t reserved[4];
} hba_cmd_sheader_t;

void ahci_deinit_ports(uintptr_t base)
{
    if (!base)
        return;

    hba_mem_t* hba = (hba_mem_t*)base;

    uint32_t ports = hba->pi;

    for (int i = 0; i < 32; i++)
    {
        if (ports & (1U << i))
        {
            hba_port_t* port =
                (hba_port_t*)((uint8_t*)hba + 0x100 + (i * 0x80));

            ahci_port_stop(port);
        }
    }
}

void ahci_deinit(cs_task *self)
{
    (void)self;

    uintptr_t ahci_base =
        ahci_get_base(0, 31, 2);

    ahci_deinit_ports(ahci_base);
}

typedef struct
{
    uint8_t  cfis[64];
    uint8_t  acmd[16];
    uint8_t  rsv[48];
    uint32_t prdt_entry[4];
} cmd_table_t;

static int ahci_cwait(volatile uint32_t* reg, uint32_t mask, int timeout)
{
    while (*reg & mask)
        if (--timeout == 0)
            return -1;
    return 0;
}

int ahci_id(hba_port_t* port, char* model_out)
{
    if (!port)
        return -1;

    // 1. stop engine safely
    port->cmd &= ~HBA_PxCMD_ST;
    port->cmd &= ~HBA_PxCMD_FRE;

    if (ahci_cwait(&port->cmd, HBA_PxCMD_CR | HBA_PxCMD_FR, 1000000))
        return -2;

    // 2. pick slot 0 (simple version)
    int slot = 0;

    hba_cmd_sheader_t* hdr =
        (hba_cmd_sheader_t*)(uintptr_t)(port->clb);
    hba_cmd_sheader_t* h = &hdr[slot];

    memset(h, 0, sizeof(hba_cmd_sheader_t));

    cmd_table_t* tbl = (cmd_table_t*)kmalloc(sizeof(cmd_table_t));
    memset(tbl, 0, sizeof(cmd_table_t));

    h->ctba = (uint32_t)(uintptr_t)tbl;
    h->flags = sizeof(cmd_table_t) / sizeof(uint32_t);

    // 3. build IDENTIFY FIS
    uint8_t* fis = tbl->cfis;

    fis[0] = 0x27;
    fis[1] = 1 << 7;
    fis[2] = 0xEC;

    // 4. buffer (DMA safe required in real system)
    uint8_t* buf = (uint8_t*)kmalloc(512);
    memset(buf, 0, 512);

    uint64_t addr = (uint64_t)buf;

    tbl->prdt_entry[0] = (uint32_t)addr;
    tbl->prdt_entry[1] = 0;
    tbl->prdt_entry[2] = 511;
    tbl->prdt_entry[3] = 0;

    h->prdtl = 1;

    // 5. issue command
    port->ci = 1 << slot;

    // 6. wait completion
    if (ahci_cwait(&port->ci, 1 << slot, 2000000))
        return -3;


    // 7. parse model string (word swap)
    uint16_t* id = (uint16_t*)buf;
    int k = 0;

    for (int i = 27; i <= 46; i++)
    {
        model_out[k++] = id[i] >> 8;
        model_out[k++] = id[i] & 0xFF;
    }

    model_out[k] = 0;

    return 0;
}

int ahci_name_identify(hba_port_t* port, char* model_out)
{
    if (!port || !model_out)
        return -1;

    // Basic sanity: no device present
    if ((port->ssts & 0x0F) != 0x03)
        return -2;

    // Stop command engine
    port->cmd &= ~HBA_PxCMD_ST;
    port->cmd &= ~HBA_PxCMD_FRE;

    while (port->cmd & ((1 << 15) | (1 << 14)));

    // Pick slot 0 (minimal version)
    int slot = 0;

    hba_cmd_sheader_t* hdr =
        (hba_cmd_sheader_t*)(uintptr_t)(port->clb);

    hba_cmd_sheader_t* h = &hdr[slot];

    memset(h, 0, sizeof(hba_cmd_sheader_t));

    // Command table (DMA)
    typedef struct
    {
        uint8_t  cfis[64];
        uint8_t  acmd[16];
        uint8_t  rsv[48];
        uint32_t prdt_entry[4];
    } cmd_table_t;

    cmd_table_t* tbl =
        (cmd_table_t*)kmalloc(sizeof(cmd_table_t));

    memset(tbl, 0, sizeof(cmd_table_t));

    h->ctba = (uint32_t)(uintptr_t)tbl;
    h->flags = (sizeof(cmd_table_t) / sizeof(uint32_t));

    // Build IDENTIFY FIS
    uint8_t* fis = tbl->cfis;

    fis[0] = 0x27;
    fis[1] = 1 << 7;
    fis[2] = 0xEC;

    // DMA buffer
    uint8_t* buf = (uint8_t*)kmalloc(512);
    memset(buf, 0, 512);

    uint64_t addr = (uint64_t)buf;

    tbl->prdt_entry[0] = (uint32_t)addr;
    tbl->prdt_entry[1] = 0;
    tbl->prdt_entry[2] = 511;
    tbl->prdt_entry[3] = 0;

    h->prdtl = 1;

    // Issue command
    port->ci = 1 << slot;

    // Wait completion (simple timeout loop)
    for (volatile int i = 0; i < 10000000; i++)
    {
        if ((port->ci & (1 << slot)) == 0)
            break;
    }

    if (port->tfd & 1)
        return -3;

    // Parse model string (words 27–46)
    uint16_t* id = (uint16_t*)buf;
    int k = 0;

    for (int i = 27; i <= 46; i++)
    {
        model_out[k++] = id[i] >> 8;
        model_out[k++] = id[i] & 0xFF;
    }

    model_out[k] = 0;

    // trim spaces
    for (int i = k - 1; i >= 0; i--)
    {
        if (model_out[i] == ' ')
            model_out[i] = 0;
        else
            break;
    }

    return 0;
}

void print_disk_name(hba_port_t* port)
{
    char model[64];

    if (ahci_name_identify(port, model) == 0)
    {
        kprintf("Disk model: %s\n", model);
    }
    else
    {
        kprintf("Failed to identify disk\n");
    }
}



void ahci_print_all_disk_names(uintptr_t ahci_base)
{
    hba_mem_t* hba = (hba_mem_t*)ahci_base;
    uint32_t ports = hba->pi;

    for (int i = 0; i < 32; i++)
    {
        if (!(ports & (1U << i)))
            continue;

        hba_port_t* port =
            (hba_port_t*)((uint8_t*)hba + 0x100 + (i * 0x80));

        char model[64] = {0};

        int res = ahci_name_identify(port, model);

        if (res == 0)
        {
            kprintf("Port %d: %s\n", i, model);
        }
        else
        {
            kprintf("Port %d: IDENTIFY failed (%d)\n", i, res);
        }
    }
}

void ahci_print_all_disks(void)
{
    uintptr_t base = ahci_get_base(0, 31, 2);

    if (!base)
    {
        kprintf("No AHCI controller found\n");
        return;
    }

    ahci_print_all_disk_names(base);
}
