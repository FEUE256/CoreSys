#pragma once

#include <drivers/pci/main.h>
#include <drivers/page/main.h>
#include <inttypes.h>
#include <drivers/serial/main.h>
#include <stdint.h>

#define NVME_CLASS      0x01
#define NVME_SUBCLASS   0x08
#define NVME_PROGIF     0x02

#define NVME_ADMIN_IDENTIFY 0x06
#define NVME_CMD_READ       0x02
#define NVME_CMD_WRITE      0x01

#define NVME_SQ_SIZE 64
#define NVME_CQ_SIZE 64

#define ALIGN_UP(x, a) (((x) + ((a)-1)) & ~((a)-1))

#define NVME_SQTDBL 0x1000
#define NVME_CQHDBL 0x1004

#define asm __asm__

/* ---------------- REGISTERS ---------------- */

typedef volatile struct
{
    uint64_t CAP;
    uint32_t VS;
    uint32_t INTMS;
    uint32_t INTMC;

    uint32_t CC;
    uint32_t RESERVED0;

    uint32_t CSTS;
    uint32_t RESERVED1;

    uint32_t AQA;

    uint64_t ASQ;
    uint64_t ACQ;
} NVMERegs;

/* ---------------- COMMANDS ---------------- */

typedef struct
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t cid;
    uint32_t nsid;
    uint64_t reserved;
    uint64_t mptr;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10_15[6];
} NVMECommand;

typedef struct
{
    uint32_t result;
    uint32_t reserved;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;
} NVMECpl;

/* ---------------- IDENTIFY ---------------- */

typedef struct
{
    uint16_t vid;
    uint16_t ssvid;
    char sn[20];
    char mn[40];
    char fr[8];
    uint8_t rab;
    uint8_t ieee[3];
    uint8_t cmic;
    uint8_t mdts;
    uint8_t reserved[4054];
} NVMEIdentifyController;

/* ---------------- DEVICE ---------------- */

typedef struct
{
    uint8_t bus;
    uint8_t dev;
    uint8_t fn;

    uintptr_t mmio;
    NVMERegs* regs;

    void* asq;
    void* acq;

    void* identify_buf;

    uint32_t sq_tail;
    uint32_t cq_head;

    uint8_t dstrd;
} NVMEDevice;

typedef struct
{
    uint16_t ms;
    uint8_t lbads;
    uint8_t rp;

} NVMELBAF;

typedef struct
{
    uint64_t nsze;
    uint64_t ncap;
    uint64_t nuse;

    uint8_t nsfeat;
    uint8_t nlbaf;
    uint8_t flbas;
    uint8_t mc;

    uint8_t dpc;
    uint8_t dps;
    uint8_t nmic;
    uint8_t rescap;

    uint8_t fpi;
    uint8_t dlfeat;

    uint16_t nawun;
    uint16_t nawupf;
    uint16_t nacwu;
    uint16_t nabsn;

    uint16_t nabo;
    uint16_t nabspf;
    uint16_t noiob;

    uint8_t nvmcap[16];   /* offset 48-63, previously missing entirely */

    uint8_t reserved0[64]; /* offset 64-127, pad up to LBAF table */

    NVMELBAF lbaf[16];    /* real spec offset = 128 */

    uint8_t reserved1[3648]; /* pad struct to 4096 total, adjust as needed */

} NVMEIdentifyNS;

NVMEDevice nvme_devices[32];
int nvme_count = 0;

/* ---------------- KMALLOC HELPERS ---------------- */

static inline uint32_t nvme_next_sq_tail(NVMEDevice* n)
{
    return (n->sq_tail + 1) % NVME_SQ_SIZE;
}

typedef struct
{
    void* original;
} nvme_align_hdr;

void* nvme_alloc_aligned(size_t size, size_t align)
{
    size_t total =
        size +
        align +
        sizeof(nvme_align_hdr);

    void* raw = kmalloc(total);

    if (!raw)
        return NULL;

    uintptr_t p =
        (uintptr_t)raw +
        sizeof(nvme_align_hdr);

    uintptr_t aligned =
        ALIGN_UP(p, align);

    nvme_align_hdr* hdr =
        (nvme_align_hdr*)
        (aligned - sizeof(nvme_align_hdr));

    hdr->original = raw;

    return (void*)aligned;
}

void nvme_free_aligned(void* ptr)
{
    if (!ptr)
        return;

    nvme_align_hdr* hdr =
        (nvme_align_hdr*)
        ((uintptr_t)ptr -
        sizeof(nvme_align_hdr));

    kfree(hdr->original);
}

/* ---------------- PCI BAR ---------------- */

uint64_t pci_read_bar64(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t offset)
{
    uint64_t low  = pci_read(bus, dev, fn, offset);
    uint64_t high = pci_read(bus, dev, fn, offset + 4);
    return (high << 32) | (low & ~0xFULL);
}

/* ---------------- WAIT READY ---------------- */

void nvme_wait_ready(NVMERegs* r)
{
    while (!(r->CSTS & 1));
}

/* ---------------- DOORBELL ---------------- */

void nvme_ring_sq(NVMEDevice* n)
{
    *(volatile uint32_t*)((uintptr_t)n->regs + NVME_SQTDBL) = n->sq_tail;
}

void nvme_ring_cq(NVMEDevice* n)
{
    *(volatile uint32_t*)((uintptr_t)n->regs + NVME_CQHDBL) = n->cq_head;
}

/* ---------------- CQ POLL ---------------- */

int nvme_poll_cq(NVMEDevice* n)
{
    NVMECpl* cq = (NVMECpl*)n->acq;

    uint32_t spins = 0;
    NVMECpl *cpl = &cq[n->cq_head];
    uint16_t status;
    int8_t phase;

    do {
        status = cpl->status;
        phase = (status >> 15) & 1;

        if (phase == 1)
            break;

        spins++;
    } while (spins < 5000000);

    if (phase < 0) 
    {
        kprintf("[NVMe] phase=%d", phase);
        kprintf("[NVMe] CQ timeout\n");
        return -1;
    }

    uint8_t sc  = (status >> 1) & 0x7F;
    uint8_t sct = (status >> 8) & 0x7;
    uint16_t cid = cq[n->cq_head].cid;

    n->cq_head = (n->cq_head + 1) % NVME_CQ_SIZE;

    if (n->cq_head == 0)
        phase ^= 1;

    asm volatile("mfence" ::: "memory");
    nvme_ring_sq(n);

    if (sc != 0) {
        kprintf("[NVMe] Error SC=%u SCT=%u CID=%u\n", sc, sct, cid);
        return -sc;
    }

    return sc;
}

/* ---------------- STRING PRINT ---------------- */

void nvme_trim_print(const char* label, const char* s, int len)
{
    char tmp[64];
    int i;

    for (i = 0; i < len; i++)
    {
        if (s[i] == ' ')
            break;
        tmp[i] = s[i];
    }

    tmp[i] = '\0';

    kprintf("%s: %s\n", label, tmp);
}

void* nvme_dma_alloc()
{
    void* p = pmm_alloc_pages(1);   // 4K page
    if (!p) return NULL;

    memset(p, 0, 4096);
    return p;
}

/* ---------------- IDENTIFY CONTROLLER ---------------- */

void nvme_identify_controller(NVMEDevice* n)
{
    n->identify_buf = kmalloc(4096);
    if (!n->identify_buf) return;

    NVMECommand* sq = (NVMECommand*)n->asq;

    for (int i = 0; i < NVME_SQ_SIZE; i++)
        sq[i] = (NVMECommand){0};

    uint32_t tail = n->sq_tail;

    sq[tail].opcode = NVME_ADMIN_IDENTIFY;
    sq[tail].nsid   = 0;
    sq[tail].cid    = tail;
    sq[tail].prp1   = (uint64_t)n->identify_buf;
    sq[tail].cdw10_15[0] = 1;

    n->sq_tail = (tail + 1) % NVME_SQ_SIZE;
    nvme_ring_sq(n);

    int status = nvme_poll_cq(n);
    if (status < 0)
    {
        kprintf("[NVMe] controller identify failed\n");
        return;
    }

    NVMEIdentifyController* id = (NVMEIdentifyController*)n->identify_buf;

    nvme_trim_print("NVMe SN", id->sn, 20);
    nvme_trim_print("NVMe MN", id->mn, 40);
    nvme_trim_print("NVMe FW", id->fr, 8);
}

/* ---------------- IDENTIFY NAMESPACE ---------------- */

void nvme_identify_namespace(
    NVMEDevice* n,
    uint32_t nsid)
{

    void* buf =
        nvme_dma_alloc();

    if (!buf)
    {
        kprintf(
            "[NVMe] identify alloc failed\n"
        );

        return;
    }

    kprintf("[NVMe] buf addr=");
    kprint_u64((uint64_t)buf);
    kprintf("\n");

    for (
        int i = 0;
        i < 4096;
        i++
    )
    {
        ((volatile uint8_t*)buf)[i] =
            0;
    }

    NVMECommand* sq =
        (NVMECommand*)n->asq;

    for (
        int i = 0;
        i < NVME_SQ_SIZE;
        i++
    )
    {
        sq[i] =
            (NVMECommand){0};
    }

    uint32_t tail = n->sq_tail;

    sq[tail] = (NVMECommand){0};
    sq[tail].opcode = NVME_ADMIN_IDENTIFY;
    sq[tail].cid    = tail;
    sq[tail].nsid   = nsid;
    sq[tail].prp1   = (uint64_t)buf;
    sq[tail].prp2   = 0;
    sq[tail].cdw10_15[0] = 0x00;  // CNS=0 Identify Namespace

    n->sq_tail = (tail + 1) % NVME_SQ_SIZE;

    asm volatile("clflush (%0)" :: "r"(buf));
    asm volatile("mfence" ::: "memory");

    nvme_ring_sq(n);

    int status = nvme_poll_cq(n);

    if (
        status < 0
    )
    {
        kprintf(
            "[NVMe] namespace identify failed\n"
        );

        nvme_free_aligned(
            buf
        );

        return;
    }

    uint8_t* b = (uint8_t*)buf;

    for (int i = 0; i < 64; i++)
    {
        kprint_u64(b[i]);
        kprintf(" ");
    }
    kprintf("\n");

    NVMEIdentifyNS* ns =
        (NVMEIdentifyNS*)buf;

    uint8_t active =
        ns->flbas &
        0x0F;

    if (
        active > 15
    )
    {
        active =
            0;
    }

    uint8_t lbads =
        ns->lbaf[
            active
        ].lbads;

    uint32_t lba_size =
        (
            lbads < 32
        )
        ?
        (
            1U <<
            lbads
        )
        :
        0;

    uint64_t total =
        ns->nsze *
        (
            uint64_t
        )
        lba_size;

    kprintf(
        "[NVMe] Namespace "
    );

    kprint_u64(
        nsid
    );

    kprintf(
        " identified\n"
    );

    kprintf(
        "[NVMe] FLBAS = "
    );

    kprint_u64(
        ns->flbas
    );

    kprintf(
        "\n"
    );

    kprintf(
        "[NVMe] Active LBA format = "
    );

    kprint_u64(
        active
    );

    kprintf(
        "\n"
    );

    kprintf(
        "[NVMe] LBADS = "
    );

    kprint_u64(
        lbads
    );

    kprintf(
        "\n"
    );

    kprintf(
        "[NVMe] LBA size = "
    );

    kprint_u64(
        lba_size
    );

    kprintf(
        " bytes\n"
    );

    kprintf(
        "[NVMe] Namespace blocks = "
    );

    kprint_u64(
        ns->nsze
    );

    kprintf(
        "\n"
    );

    kprintf(
        "[NVMe] Capacity = "
    );

    kprint_u64(
        total
    );

    kprintf(
        " bytes\n"
    );

    kprintf(
        "[NVMe] Capacity = "
    );

    kprint_u64(
        total /
        1024 /
        1024
    );

    kprintf(
        " MiB\n"
    );

    kprintf("[NVMe] LBAs = %d\n", total / lba_size);

    nvme_free_aligned(
        buf
    );
}

static inline void nvme_debug_cmd(uint64_t prp1, uint32_t nsid, uint64_t lba)
{
    kprintf("[NVMe] PRP1=%p\n", (void*)prp1);
    kprintf("[NVMe] NSID=%u\n", nsid);
    kprintf("[NVMe] LBA=");
    kprint_u64((unsigned long long)lba);

    kprintf("\n[NVMe] PRP_ALIGN_4K=");
    kprint_u64(prp1 & 0xFFF);
    kprintf("\n[NVMe] PRP_LOW32=%08x\n", (uint32_t)prp1);
}

/* ---------------- READ ---------------- */
int nvme_read_sector(NVMEDevice* n, uint32_t nsid, uint64_t lba, void* buf)
{
    if (!n || !buf)
        return -1;

    /* Ensure valid device state */
    if (!n->asq || !n->acq)
        return -2;

    NVMECommand* sq = (NVMECommand*)n->asq;

    /* Clear command slot (important for QEMU/NVMe sanity) */
    NVMECommand cmd = {0};

    uint32_t tail = n->sq_tail;

    /* ---------------- COMMAND SETUP ---------------- */

    cmd.opcode = NVME_CMD_READ;
    cmd.nsid   = nsid;

    /* Unique CID (NOT strictly required but safer than tail reuse) */
    cmd.cid = tail;

    /* PRP (assumes identity-mapped or correct phys mapping) */
    cmd.prp1 = (uint64_t)buf;

    /* LBA encoding (correct NVMe layout) */
    cmd.cdw10_15[0] = (uint32_t)(lba & 0xFFFFFFFF);      // SLBA low
    cmd.cdw10_15[1] = (uint32_t)(lba >> 32);             // SLBA high

    /* Number of logical blocks - 1 (1 block = 0) */
    cmd.cdw10_15[2] = 0;

    /* ---------------- DEBUG ---------------- */
    nvme_debug_cmd(cmd.prp1, cmd.nsid, lba);

    /* ---------------- SUBMIT ---------------- */

    sq[tail] = cmd;

    n->sq_tail = (tail + 1) % NVME_SQ_SIZE;

    /* Ring doorbell */
    nvme_ring_sq(n);

    /* Wait for completion */
    return nvme_poll_cq(n);
}

static inline uint64_t nvme_phys(void* v)
{
    return virt_to_phys(v);   // MUST be identity-mapped or correct paging translation
}

static inline int nvme_is_page_aligned(void* v)
{
    return ((uint64_t)v & 0xFFF) == 0;
}


/* ---------------- INIT ---------------- */

void nvme_init_controller(NVMEDevice* n)
{
    NVMERegs* r = n->regs;

    if (!r)
        return;

    /* ---------------- RESET CONTROLLER ---------------- */

    r->CC &= ~1;                 // disable EN
    while (r->CSTS & 1);        // wait until RDY = 0

    n->sq_tail = 0;
    n->cq_head = 0;

    /* ---------------- ALLOCATE QUEUES (ONLY ONCE) ---------------- */

    n->asq = pmm_alloc_pages(1);
    n->acq = pmm_alloc_pages(1);

    if (!n->asq || !n->acq)
    {
        kprintf("[NVMe] queue alloc failed\n");
        return;
    }

    memset(n->asq, 0, 4096);
    memset(n->acq, 0, 4096);

    /* ---------------- PROGRAM QUEUE REGISTERS ---------------- */

    r->ASQ = (uint64_t)n->asq;
    r->ACQ = (uint64_t)n->acq;

    r->AQA =
        ((NVME_SQ_SIZE - 1) << 16) |
        (NVME_CQ_SIZE - 1);

    /* ---------------- SET CC (SAFE ORDER) ---------------- */

    r->CC = 0;
    while (r->CSTS & 1);

    r->CC = 0;

    /* ensure controller is fully reset */
    while (r->CSTS & 1);

    /* correct NVMe queue entry sizes */
    uint32_t iosqes = 6;   // 64 bytes (2^6)
    uint32_t iocqes = 4;   // 16 bytes (STANDARD for completion queue!)

    r->CC =
        (iocqes << 20) |   // IOCQES
        (iosqes << 16) |   // IOSQES
        1;                 // EN
    /* ---------------- WAIT READY ---------------- */

    nvme_wait_ready(r);

    kprintf("[NVMe] controller ready\n");

    /* ---------------- IDENTIFY CONTROLLER ---------------- */

    nvme_identify_controller(n);

    /* ---------------- IDENTIFY NAMESPACE ---------------- */

    for (uint32_t i = 1; i <= 1; i++)
        nvme_identify_namespace(n, i);
}

/* ---------------- PROBE ---------------- */

void nvme_probe(uint8_t bus, uint8_t dev, uint8_t fn)
{
    if (nvme_count >= 32) return;

    uint64_t bar0 = pci_read_bar64(bus, dev, fn, 0x10);
    if (!bar0) return;

    NVMEDevice* n = &nvme_devices[nvme_count];

    n->bus = bus;
    n->dev = dev;
    n->fn  = fn;

    n->mmio = (uintptr_t)bar0;
    n->regs = (NVMERegs*)n->mmio;

    kprintf("NVMe %u:%u:%u\n", bus, dev, fn);

    kprintf("Version=%u.%u\n",
        (n->regs->VS >> 16) & 0xFFFF,
        (n->regs->VS >> 8) & 0xFF);

    kprintf("CAP=%x %x\n",
        (uint32_t)(n->regs->CAP >> 32),
        (uint32_t)n->regs->CAP);

    nvme_init_controller(n);

    nvme_count++;
}

void nvme_deinit_controller(NVMEDevice* n)
{
    if (!n)
        return;

    if (n->regs)
    {
        n->regs->CC &= ~1;

        while (n->regs->CSTS & 1);
    }

    if (n->identify_buf)
    {
        kfree(n->identify_buf);
        n->identify_buf = NULL;
    }

    if (n->asq)
    {
        nvme_free_aligned(n->asq);
        n->asq = NULL;
    }

    if (n->acq)
    {
        nvme_free_aligned(n->acq);
        n->acq = NULL;
    }

    n->sq_tail = 0;
    n->cq_head = 0;
}

/* ---------------- SCAN ---------------- */

void nvme_scan()
{
    nvme_count = 0;

    for (uint16_t bus = 0; bus < 256; bus++)
    for (uint8_t dev = 0; dev < 32; dev++)
    for (uint8_t fn = 0; fn < 8; fn++)
    {
        uint32_t class_reg = pci_read(bus, dev, fn, 0x08);

        uint8_t class    = (class_reg >> 24) & 0xFF;
        uint8_t subclass = (class_reg >> 16) & 0xFF;
        uint8_t progif   = (class_reg >> 8) & 0xFF;

        if (class == NVME_CLASS &&
            subclass == NVME_SUBCLASS &&
            progif == NVME_PROGIF)
        {
            nvme_probe(bus, dev, fn);
        }
    }

    kprintf("NVMe count=%u\n", nvme_count);
}

void nvme_512_dump(NVMEDevice* n, uint32_t nsid, uint64_t lba)
{
    uint8_t* buf = (uint8_t*)pmm_alloc_pages(1);
    if (!buf)
    {
        kprintf("[NVMe] dump buffer alloc failed\n");
        return;
    }

    int status = nvme_read_sector(n, nsid, lba, buf);
    if (status > 0)
    {
        kprintf("[NVMe] read failed (status=%d)\n", status);
        pmm_free_pages(buf, 1);
        return;
    }

    kprintf("status=%d buf=%p phys=%p \n", status, buf, (void*)virt_to_phys(buf));

    kprintf("==== NVMe LBA ");
    kprint_u64(lba);
    kprintf(" (512 bytes) ====\n");

    for (int i = 0; i < 512; i += 16)
    {
        kprintf("%04x  ", i);
        for (int j = 0; j < 16; j++)
            kprintf("%02x ", buf[i + j]);
        kprintf(" | ");
        for (int j = 0; j < 16; j++)
        {
            uint8_t c = buf[i + j];
            kprintf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        kprintf("\n");
    }

    pmm_free_pages(buf, 1);
}

/* Wrapper: drive index -> NVMe device */
NVMEDevice* nvme_get_device(int drive)
{
    if (drive < 0 || drive >= nvme_count)
        return NULL;

    return &nvme_devices[drive];
}

/* High-level wrapper: dump using drive index */
void nvme_dump_drive_lba(int drive, uint32_t nsid, uint64_t lba)
{
    NVMEDevice* dev = nvme_get_device(drive);
    if (!dev)
    {
        kprintf("[NVMe] invalid drive index %d\n", drive);
        return;
    }

    nvme_512_dump(dev, nsid, lba);
}

/* ---------------- WRITE ---------------- */

int nvme_write_sector(NVMEDevice* n, uint32_t nsid, uint64_t lba, void* buf)
{
    if (!n || !buf)
        return -1;

    NVMECommand cmd;
    memset(&cmd, 0, sizeof(NVMECommand));
    uint32_t tail = n->sq_tail;

    cmd.opcode = NVME_CMD_WRITE;
    cmd.cid    = tail;
    cmd.nsid   = nsid;

    /* MUST be physical address */
    cmd.prp1 = virt_to_phys(buf);

    /* NVMe spec: CDW10 = LBA low */
    cmd.cdw10_15[0] = (uint32_t)(lba & 0xFFFFFFFF);

    /* CDW11 = LBA high */
    cmd.cdw10_15[1] = (uint32_t)(lba >> 32);

    /* CDW12 = number of blocks - 1 (0 = 1 block) */
    cmd.cdw10_15[2] = 0;

    /* memory barrier BEFORE doorbell */
    asm volatile("mfence" ::: "memory");

    n->sq_tail = (tail + 1) % NVME_SQ_SIZE;

    nvme_ring_sq(n);

    return nvme_poll_cq(n);
}

int nvme_write_sector_data(int drive, int nsid, int lba, int data)
{
    if (!drive)
        return -1;
    
    NVMEDevice* dev = nvme_get_device(drive);

    if (!dev)
        return -2;

    void* buf = nvme_alloc_aligned(4096, 4096);
    if (!buf)
        return -1;

    for (int i = 0; i < 4096; i++)
        ((uint8_t*)buf)[i] = data;

    int status = nvme_write_sector(dev, nsid, lba, buf);

    nvme_free_aligned(buf);

    return status;
}

