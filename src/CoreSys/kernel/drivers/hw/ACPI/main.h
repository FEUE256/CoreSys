// PCI/PCIe ACPI Driver (Approved by FÈUE in the CoreSys kernel hardware drivers collation)
#pragma once

#include <drivers/serial/main.h>
#include <drivers/pci/main.h>
#include <drivers/sf/main.h>
#include <drivers/task/main.h>
#include <drivers/halt/main.h>

#include <kernel/mem.h>

#include <stdint.h>

/*
    Parsed information
*/

uint32_t acpi_cpu_count = 0;

uint32_t acpi_ioapic_count = 0;

uint32_t acpi_iso_count = 0;

typedef struct
{
    uint16_t vendor;
    uint16_t device;

    uint8_t bus;
    uint8_t dev;
    uint8_t fun;

    uint16_t pm_base;

    uint8_t enabled;

} ACPI_DEVICE;

static ACPI_DEVICE acpi;


/*
    ACPI Structures
    CoreSys
*/

#pragma pack(push,1)

/*
    Root System Description Pointer
    ACPI 2.0+
*/
typedef struct
{
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;

    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;

    uint8_t  extended_checksum;
    uint8_t  reserved[3];

} ACPI_RSDP;


/*
    Standard ACPI SDT Header
*/
typedef struct
{
    char     signature[4];

    uint32_t length;

    uint8_t  revision;
    uint8_t  checksum;

    char     oem_id[6];

    char     oem_table_id[8];

    uint32_t oem_revision;

    uint32_t creator_id;
    uint32_t creator_revision;

} ACPI_SDT;


/*
    RSDT
*/
typedef struct
{
    ACPI_SDT header;

} ACPI_RSDT;


/*
    XSDT
*/
typedef struct
{
    ACPI_SDT header;

} ACPI_XSDT;


/*
    Fixed ACPI Description Table
*/
typedef struct
{
    ACPI_SDT header;

    uint32_t firmware_ctrl;
    uint32_t dsdt;

    uint8_t  reserved0;

    uint8_t  preferred_pm_profile;

    uint16_t sci_int;

    uint32_t smi_cmd;

    uint8_t  acpi_enable;
    uint8_t  acpi_disable;

    uint8_t  s4bios_req;
    uint8_t  pstate_cnt;

    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;

    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;

    uint32_t pm2_cnt_blk;

    uint32_t pm_tmr_blk;

    uint32_t gpe0_blk;
    uint32_t gpe1_blk;

    uint8_t  pm1_evt_len;
    uint8_t  pm1_cnt_len;

    uint8_t  pm2_cnt_len;
    uint8_t  pm_tmr_len;

    uint8_t  gpe0_blk_len;
    uint8_t  gpe1_blk_len;

    uint8_t  gpe1_base;

    uint8_t  cstate_control;

    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;

    uint16_t flush_size;
    uint16_t flush_stride;

    uint8_t  duty_offset;
    uint8_t  duty_width;

    uint8_t  day_alarm;
    uint8_t  month_alarm;

    uint8_t  century;

    uint16_t boot_arch_flags;

    uint8_t  reserved1;

    uint32_t flags;

    uint64_t reset_reg;
    uint8_t  reset_value;

    uint8_t  reserved2[3];

    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;

    uint64_t x_pm1a_evt_blk;
    uint64_t x_pm1b_evt_blk;

    uint64_t x_pm1a_cnt_blk;
    uint64_t x_pm1b_cnt_blk;

} ACPI_FADT;


/*
    Multiple APIC Description Table
*/
typedef struct
{
    ACPI_SDT header;

    uint32_t lapic_address;

    uint32_t flags;

} ACPI_MADT;


/*
    Differentiated System Description Table
*/
typedef struct
{
    ACPI_SDT header;

    uint8_t aml[];

} ACPI_DSDT;


/*
    Generic MADT Entry
*/
typedef struct
{
    uint8_t type;
    uint8_t length;

} ACPI_MADT_ENTRY;


/*
    Processor Local APIC
*/
typedef struct
{
    ACPI_MADT_ENTRY header;

    uint8_t acpi_processor_id;
    uint8_t apic_id;

    uint32_t flags;

} ACPI_MADT_LAPIC;


/*
    IO APIC
*/
typedef struct
{
    ACPI_MADT_ENTRY header;

    uint8_t ioapic_id;
    uint8_t reserved;

    uint32_t ioapic_address;
    uint32_t gsi_base;

} ACPI_MADT_IOAPIC;


/*
    Interrupt Source Override
*/
typedef struct
{
    ACPI_MADT_ENTRY header;

    uint8_t bus;
    uint8_t source;

    uint32_t gsi;

    uint16_t flags;

} ACPI_MADT_ISO;

#pragma pack(pop)


/*
    Global ACPI pointers
*/

ACPI_RSDP *acpi_rsdp = 0;

ACPI_RSDT *acpi_rsdt = 0;

ACPI_XSDT *acpi_xsdt = 0;

ACPI_FADT *acpi_fadt = 0;

ACPI_DSDT *acpi_dsdt = 0;

ACPI_MADT *acpi_madt = 0;

/*
    Detect ACPI PCI device
*/
int acpi_detect(void)
{
    for(uint16_t bus = 0; bus < 256; bus++)
    {
        for(uint8_t dev = 0; dev < 32; dev++)
        {
            for(uint8_t fun = 0; fun < 8; fun++)
            {
                uint16_t vendor =
                    pci_read16(
                        bus,
                        dev,
                        fun,
                        0x00
                    );

                if(vendor == 0xFFFF)
                    continue;

                uint8_t class =
                    pci_read8(
                        bus,
                        dev,
                        fun,
                        0x0B
                    );

                uint8_t subclass =
                    pci_read8(
                        bus,
                        dev,
                        fun,
                        0x0A
                    );

                /*
                    ACPI Bridge
                    Bridge Device
                    Subclass 0x80
                */
                if(class == 0x06 &&
                   subclass == 0x80)
                {
                    acpi.vendor = vendor;

                    acpi.device =
                        pci_read16(
                            bus,
                            dev,
                            fun,
                            0x02
                        );

                    acpi.bus = bus;
                    acpi.dev = dev;
                    acpi.fun = fun;

                    acpi.pm_base = 0;
                    acpi.enabled = 0;

                    return 1;
                }
            }
        }
    }

    return 0;
}

/*
    Get PMBASE
*/
uint16_t acpi_get_pm_base(void)
{
    /*
        Intel chipsets usually store PMBASE
        in config register 0x40.
    */

    acpi.pm_base =
        pci_read16(
            acpi.bus,
            acpi.dev,
            acpi.fun,
            0x40
        ) & 0xFFF0;

    return acpi.pm_base;
}

/*
    Enable ACPI
*/
void acpi_enable(void)
{
    uint16_t pmcnt =
        inw(
            acpi.pm_base + 4
        );

    pmcnt |= 1;

    outw(
        acpi.pm_base + 4,
        pmcnt
    );

    acpi.enabled = 1;
}

/*
    Disable ACPI
*/
void acpi_disable(void)
{
    uint16_t pmcnt =
        inw(
            acpi.pm_base + 4
        );

    pmcnt &= ~1;

    outw(
        acpi.pm_base + 4,
        pmcnt
    );

    acpi.enabled = 0;
}

/*
    Check ACPI state
*/
int acpi_is_enabled(void)
{
    uint16_t pmcnt =
        inw(
            acpi.pm_base + 4
        );

    return (pmcnt & 1);
}

/*
    Dump ACPI information
*/
void acpi_dump(void)
{
    kprintf(
        "========== ACPI ==========\n"
    );

    kprintf(
        "Vendor: %u\n",
        acpi.vendor
    );

    kprintf(
        "Device: %u\n",
        acpi.device
    );

    kprintf(
        "Bus: %u\n",
        acpi.bus
    );

    kprintf(
        "Device: %u\n",
        acpi.dev
    );

    kprintf(
        "Function: %u\n",
        acpi.fun
    );

    kprintf(
        "PMBASE: 0x%x\n",
        acpi.pm_base
    );

    kprintf(
        "Enabled: %u\n",
        acpi_is_enabled()
    );

    kprintf(
        "==========================\n"
    );
}


uint8_t acpi_get_sci_irq(void)
{
    /*
        Intel PIIX4/ICH:
        SCI Interrupt Register (PCI config 0x3C)
    */

    return pci_read8(
        acpi.bus,
        acpi.dev,
        acpi.fun,
        0x3C
    );
}

uint16_t acpi_get_pm1_control(void)
{
    /*
        PM1 Control Register
        PMBASE + 0x04
    */

    return inw(
        acpi.pm_base + 0x04
    );
}

uint16_t acpi_get_pm1_status(void)
{
    /*
        PM1 Status Register
        PMBASE + 0x00
    */

    return inw(
        acpi.pm_base + 0x00
    );
}

uint32_t acpi_get_pm_timer(void)
{
    /*
        ACPI Power Management Timer
        PMBASE + 0x08
    */

    return inl(
        acpi.pm_base + 0x08
    );
}

void acpi_clear_events(void)
{
    /*
        Clear PM1 event bits.
        Writing 1 clears the corresponding bit.
    */

    outw(
        acpi.pm_base + 0x00,
        0xFFFF
    );
}

/*
    Compare ACPI signature
*/
int acpi_signature(
    void *ptr,
    const char *sig
)
{
    char *s = ptr;

    for(int i = 0; sig[i]; i++)
    {
        if(s[i] != sig[i])
            return 0;
    }

    return 1;
}

BOOT_INFO *boot =
    (BOOT_INFO*)KSP;


/*
    Find RSDP
    From UEFI BOOT_INFO
*/
int acpi_find_rsdp(void)
{
    boot =
        (BOOT_INFO*)KSP;


    if(!boot->acpi_available)
    {
        kprintf("[ACPI] Not available\n");
        return 0;
    }


    if(!boot->rsdp)
    {
        kprintf("[ACPI] Missing RSDP\n");
        return 0;
    }


    acpi_rsdp =
        (ACPI_RSDP*)
        boot->rsdp;

    return 1;
}



/*
    Find RSDT
*/
int acpi_find_rsdt(void)
{
    boot =
        (BOOT_INFO*)KSP;


    if(!acpi_rsdp)
        return 0;


    if(boot->rsdt)
    {
        acpi_rsdt =
            (ACPI_RSDT*)
            boot->rsdt;

        return 1;
    }


    if(acpi_rsdp->rsdt_address)
    {
        acpi_rsdt =
            (ACPI_RSDT*)
            (uint64_t)
            acpi_rsdp->rsdt_address;

        return 1;
    }


    return 0;
}



/*
    Find XSDT
*/
int acpi_find_xsdt(void)
{
    boot =
        (BOOT_INFO*)KSP;


    if(!acpi_rsdp)
        return 0;


    if(boot->xsdt)
    {
        acpi_xsdt =
            (ACPI_XSDT*)
            boot->xsdt;

        return 1;
    }


    if(acpi_rsdp->revision < 2)
        return 0;


    if(acpi_rsdp->xsdt_address)
    {
        acpi_xsdt =
            (ACPI_XSDT*)
            acpi_rsdp->xsdt_address;

        return 1;
    }


    return 0;
}



/*
    Search ACPI table
*/
static ACPI_SDT*
acpi_find_table(
    ACPI_SDT *root,
    const char *name,
    int xsdt
)
{
    if(!root)
        return 0;


    uint32_t entries =
        (root->length -
        sizeof(ACPI_SDT))
        /
        (xsdt ? 8 : 4);



    uint8_t *ptr =
        (uint8_t*)root +
        sizeof(ACPI_SDT);



    for(uint32_t i = 0; i < entries; i++)
    {
        uint64_t addr;


        if(xsdt)
            addr =
                *(uint64_t*)ptr;
        else
            addr =
                *(uint32_t*)ptr;



        ACPI_SDT *table =
            (ACPI_SDT*)addr;



        if(acpi_signature(
                table,
                name))
        {
            return table;
        }


        ptr += xsdt ? 8 : 4;
    }


    return 0;
}



/*
    Find FADT
*/
int acpi_find_fadt(void)
{
    ACPI_SDT *root =
        acpi_xsdt ?
        (ACPI_SDT*)acpi_xsdt :
        (ACPI_SDT*)acpi_rsdt;



    if(!root)
        return 0;



    acpi_fadt =
        (ACPI_FADT*)
        acpi_find_table(
            root,
            "FACP",
            acpi_xsdt != 0
        );


    return acpi_fadt != 0;
}



/*
    Find DSDT
*/
int acpi_find_dsdt(void)
{
    if(!acpi_fadt)
        return 0;



    uint32_t dsdt_addr =
        *(uint32_t*)
        (
            (uint8_t*)acpi_fadt
            +
            0x28
        );



    acpi_dsdt =
        (ACPI_DSDT*)
        (uint64_t)
        dsdt_addr;



    return acpi_dsdt != 0;
}



/*
    Find MADT
*/
int acpi_find_madt(void)
{   
    ACPI_SDT *root =
        acpi_xsdt ?
        (ACPI_SDT*)acpi_xsdt :
        (ACPI_SDT*)acpi_rsdt;

    if(!root)
        return 0;



    acpi_madt =
        (ACPI_MADT*)
        acpi_find_table(
            root,
            "APIC",
            acpi_xsdt != 0
        );


    return acpi_madt != 0;
}



/*
    Dumps
*/

void acpi_dump_rsdp(void)
{
    if(!acpi_rsdp)
        return;


    kprintf("===== RSDP =====\n");


    kprintf("Revision: ");
    kprint_u64(
        acpi_rsdp->revision
    );


    kprintf("\nRSDT: 0x");
    kprint_u64(
        acpi_rsdp->rsdt_address
    );


    if (acpi_rsdp->revision >= 2)
    {
        kprintf("XSDT: 0x");
        kprint_u64(acpi_rsdp->xsdt_address);
        kprintf("\n");
    }
    else
    {
        kprintf("XSDT: Not supported (ACPI 1.0)\n");
    }


    kprintf("\n");
}



void acpi_dump_rsdt(void)
{
    if(!acpi_rsdt)
        return;


    kprintf("RSDT found\n");


    kprintf("Length: ");
    kprint_u64(
        acpi_rsdt->header.length
    );


    kprintf("\n");
}



void acpi_dump_xsdt(void)
{
    if(!acpi_xsdt)
        return;


    kprintf("XSDT found\n");


    kprintf("Length: ");
    kprint_u64(
        acpi_xsdt->header.length
    );


    kprintf("\n");
}



void acpi_dump_fadt(void)
{
    if(!acpi_fadt)
        return;


    kprintf("FADT found\n");


    kprintf("Length: ");
    kprint_u64(
        acpi_fadt->header.length
    );


    kprintf("\n");
}



void acpi_dump_dsdt(void)
{
    if(!acpi_dsdt)
        return;


    kprintf("DSDT found\n");


    kprintf("Length: ");
    kprint_u64(
        acpi_dsdt->header.length
    );


    kprintf("\n");
}



void acpi_dump_madt(void)
{
    if(!acpi_madt)
        return;


    kprintf("MADT found\n");


    kprintf("Length: ");
    kprint_u64(
        acpi_madt->header.length
    );


    kprintf("\n");
}

/*
    Helper
*/

static inline int
acpi_signaturek(
    const void *table,
    const char *sig
)
{
    return memcmp(
        (void*)table,
        (void*)sig,
        4
    ) == 0;
}

/*
    --------------------------------------------------
    MADT Parser
    CoreSys
    --------------------------------------------------
*/

static void acpi_parse_lapic(
    ACPI_MADT_LAPIC *lapic
);

static void acpi_parse_ioapic(
    ACPI_MADT_IOAPIC *ioapic
);

static void acpi_parse_iso(
    ACPI_MADT_ISO *iso
);


/*
    Parse entire MADT
*/
void acpi_parse_madt(void)
{
    if(!acpi_madt)
    {
        kprintf("[ACPI] MADT not found\n");
        return;
    }

    acpi_cpu_count    = 0;
    acpi_ioapic_count = 0;
    acpi_iso_count    = 0;

    kprintf("========== MADT ==========\n");

    kprintf("Local APIC Address: 0x");
    kprint_u64(acpi_madt->lapic_address);
    kprintf("\n");

    kprintf("Flags: 0x");
    kprint_u64(acpi_madt->flags);
    kprintf("\n\n");


    /*
        First MADT entry
    */
    uint8_t *ptr =
        (uint8_t*)acpi_madt +
        sizeof(ACPI_MADT);


    /*
        End of table
    */
    uint8_t *end =
        (uint8_t*)acpi_madt +
        acpi_madt->header.length;


    while(ptr < end)
    {
        ACPI_MADT_ENTRY *entry =
            (ACPI_MADT_ENTRY*)ptr;

        /*
            Invalid entry
        */
        if(entry->length == 0)
            break;

        switch(entry->type)
        {
            /*
                Processor Local APIC
            */
            case 0:

                acpi_parse_lapic(
                    (ACPI_MADT_LAPIC*)entry
                );

                break;


            /*
                IO APIC
            */
            case 1:

                acpi_parse_ioapic(
                    (ACPI_MADT_IOAPIC*)entry
                );

                break;


            /*
                Interrupt Source Override
            */
            case 2:

                acpi_parse_iso(
                    (ACPI_MADT_ISO*)entry
                );

                break;


            /*
                NMI Source
            */
            case 3:

                kprintf(
                    "[MADT] NMI Source\n"
                );

                break;


            /*
                Local APIC NMI
            */
            case 4:

                kprintf(
                    "[MADT] Local APIC NMI\n"
                );

                break;


            /*
                Local APIC Address Override
            */
            case 5:

                kprintf(
                    "[MADT] LAPIC Address Override\n"
                );

                break;


            /*
                IO SAPIC
            */
            case 6:

                kprintf(
                    "[MADT] IO SAPIC\n"
                );

                break;


            /*
                Local SAPIC
            */
            case 7:

                kprintf(
                    "[MADT] Local SAPIC\n"
                );

                break;


            default:

                kprintf(
                    "[MADT] Unknown Entry Type "
                );

                kprint_u64(entry->type);

                kprintf("\n");

                break;
        }

        ptr += entry->length;
    }


    kprintf("\n");

    kprintf("CPU Count: ");
    kprint_u64(acpi_cpu_count);

    kprintf("\nIOAPIC Count: ");
    kprint_u64(acpi_ioapic_count);

    kprintf("\nISO Count: ");
    kprint_u64(acpi_iso_count);

    kprintf("\n==========================\n");
}

/*
    --------------------------------------------------
    Parse Processor Local APIC Entry
    MADT Type 0
    --------------------------------------------------
*/

void acpi_parse_lapic(
    ACPI_MADT_LAPIC *lapic
)
{
    if(!lapic)
        return;

    acpi_cpu_count++;

    kprintf("[CPU ");

    kprint_u64(acpi_cpu_count - 1);

    kprintf("]\n");

    kprintf("ACPI Processor ID : ");
    kprint_u64(lapic->acpi_processor_id);

    kprintf("\n");

    kprintf("APIC ID           : ");
    kprint_u64(lapic->apic_id);

    kprintf("\n");

    kprintf("Flags             : 0x");
    kprint_u64(lapic->flags);

    kprintf("\n");

    if(lapic->flags & 1)
    {
        kprintf("Status            : Enabled\n");
    }
    else
    {
        kprintf("Status            : Disabled\n");
    }

    if(lapic->flags & 2)
    {
        kprintf("Online Capable    : Yes\n");
    }
    else
    {
        kprintf("Online Capable    : No\n");
    }

    kprintf("\n");
}


/*
    --------------------------------------------------
    Optional helper
    --------------------------------------------------
*/

void acpi_dump_cpus(void)
{
    if(!acpi_madt)
    {
        kprintf("[ACPI] MADT not loaded\n");
        return;
    }

    kprintf("========== CPUs ==========\n");

    uint8_t *ptr =
        (uint8_t*)acpi_madt +
        sizeof(ACPI_MADT);

    uint8_t *end =
        (uint8_t*)acpi_madt +
        acpi_madt->header.length;

    while(ptr < end)
    {
        ACPI_MADT_ENTRY *entry =
            (ACPI_MADT_ENTRY*)ptr;

        if(entry->length == 0)
            break;

        if(entry->type == 0)
        {
            acpi_parse_lapic(
                (ACPI_MADT_LAPIC*)entry
            );
        }

        ptr += entry->length;
    }

    kprintf("Total CPUs: ");
    kprint_u64(acpi_cpu_count);

    kprintf("\n==========================\n");
}

/*
    --------------------------------------------------
    Parse IO APIC Entry
    MADT Type 1
    --------------------------------------------------
*/

void acpi_parse_ioapic(
    ACPI_MADT_IOAPIC *ioapic
)
{
    if(!ioapic)
        return;

    acpi_ioapic_count++;

    kprintf("[IOAPIC ");

    kprint_u64(acpi_ioapic_count - 1);

    kprintf("]\n");

    kprintf("IOAPIC ID         : ");
    kprint_u64(ioapic->ioapic_id);

    kprintf("\n");

    kprintf("MMIO Address      : 0x");
    kprint_u64(ioapic->ioapic_address);

    kprintf("\n");

    kprintf("GSI Base          : ");
    kprint_u64(ioapic->gsi_base);

    kprintf("\n\n");
}


/*
    --------------------------------------------------
    IOAPIC Register Access
    --------------------------------------------------
*/

#define IOAPIC_REGSEL   0x00
#define IOAPIC_WINDOW   0x10

static inline uint32_t
ioapic_read(
    volatile uint32_t *base,
    uint8_t reg
)
{
    base[
        IOAPIC_REGSEL / 4
    ] = reg;

    return base[
        IOAPIC_WINDOW / 4
    ];
}


static inline void
ioapic_write(
    volatile uint32_t *base,
    uint8_t reg,
    uint32_t value
)
{
    base[
        IOAPIC_REGSEL / 4
    ] = reg;

    base[
        IOAPIC_WINDOW / 4
    ] = value;
}


/*
    --------------------------------------------------
    Dump IOAPIC Registers
    --------------------------------------------------
*/

void acpi_dump_ioapic(
    ACPI_MADT_IOAPIC *ioapic
)
{
    if(!ioapic)
        return;

    volatile uint32_t *mmio =
        (volatile uint32_t*)
        (uint64_t)
        ioapic->ioapic_address;

    uint32_t version =
        ioapic_read(
            mmio,
            1
        );

    uint32_t id =
        ioapic_read(
            mmio,
            0
        );

    uint32_t arbitration =
        ioapic_read(
            mmio,
            2
        );

    kprintf("========== IOAPIC ==========\n");

    kprintf("Address : 0x");
    kprint_u64(ioapic->ioapic_address);

    kprintf("\n");

    kprintf("ID      : ");
    kprint_u64(
        (id >> 24) & 0xF
    );

    kprintf("\n");

    kprintf("Version : ");
    kprint_u64(
        version & 0xFF
    );

    kprintf("\n");

    kprintf("Max IRQ : ");
    kprint_u64(
        ((version >> 16) & 0xFF) + 1
    );

    kprintf("\n");

    kprintf("ARB ID  : ");
    kprint_u64(
        (arbitration >> 24) & 0xF
    );

    kprintf("\n");

    kprintf("============================\n");
}


/*
    --------------------------------------------------
    Find first IOAPIC
    --------------------------------------------------
*/

ACPI_MADT_IOAPIC*
acpi_get_first_ioapic(void)
{
    if(!acpi_madt)
        return 0;

    uint8_t *ptr =
        (uint8_t*)acpi_madt +
        sizeof(ACPI_MADT);

    uint8_t *end =
        (uint8_t*)acpi_madt +
        acpi_madt->header.length;

    while(ptr < end)
    {
        ACPI_MADT_ENTRY *entry =
            (ACPI_MADT_ENTRY*)ptr;

        if(entry->length == 0)
            break;

        if(entry->type == 1)
            return
                (ACPI_MADT_IOAPIC*)entry;

        ptr += entry->length;
    }

    return 0;
}

/*
    --------------------------------------------------
    Parse Interrupt Source Override
    MADT Type 2
    --------------------------------------------------
*/

void acpi_parse_iso(
    ACPI_MADT_ISO *iso
)
{
    if(!iso)
        return;

    acpi_iso_count++;

    kprintf("[ISO ");

    kprint_u64(acpi_iso_count - 1);

    kprintf("]\n");

    kprintf("Bus              : ");
    kprint_u64(iso->bus);

    kprintf("\n");

    kprintf("IRQ Source       : ");
    kprint_u64(iso->source);

    kprintf("\n");

    kprintf("GSI              : ");
    kprint_u64(iso->gsi);

    kprintf("\n");

    kprintf("Flags            : 0x");
    kprint_u64(iso->flags);

    kprintf("\n");


    /*
        Polarity
    */

    kprintf("Polarity         : ");

    switch(iso->flags & 0x3)
    {
        case 0:
            kprintf("Bus Default");
            break;

        case 1:
            kprintf("Active High");
            break;

        case 3:
            kprintf("Active Low");
            break;

        default:
            kprintf("Reserved");
            break;
    }

    kprintf("\n");


    /*
        Trigger Mode
    */

    kprintf("Trigger          : ");

    switch((iso->flags >> 2) & 0x3)
    {
        case 0:
            kprintf("Bus Default");
            break;

        case 1:
            kprintf("Edge");
            break;

        case 3:
            kprintf("Level");
            break;

        default:
            kprintf("Reserved");
            break;
    }

    kprintf("\n\n");
}


/*
    --------------------------------------------------
    Dump all ISOs
    --------------------------------------------------
*/

void acpi_dump_iso(void)
{
    if(!acpi_madt)
    {
        kprintf("[ACPI] MADT not loaded\n");
        return;
    }

    kprintf("========== Interrupt Source Overrides ==========\n");

    uint8_t *ptr =
        (uint8_t*)acpi_madt +
        sizeof(ACPI_MADT);

    uint8_t *end =
        (uint8_t*)acpi_madt +
        acpi_madt->header.length;

    while(ptr < end)
    {
        ACPI_MADT_ENTRY *entry =
            (ACPI_MADT_ENTRY*)ptr;

        if(entry->length == 0)
            break;

        if(entry->type == 2)
        {
            acpi_parse_iso(
                (ACPI_MADT_ISO*)entry
            );
        }

        ptr += entry->length;
    }

    kprintf("Total ISO Entries: ");
    kprint_u64(acpi_iso_count);

    kprintf("\n===============================================\n");
}


/*
    --------------------------------------------------
    Convert Legacy IRQ -> GSI
    --------------------------------------------------
*/

uint32_t acpi_irq_to_gsi(
    uint8_t irq
)
{
    if(!acpi_madt)
        return irq;

    uint8_t *ptr =
        (uint8_t*)acpi_madt +
        sizeof(ACPI_MADT);

    uint8_t *end =
        (uint8_t*)acpi_madt +
        acpi_madt->header.length;

    while(ptr < end)
    {
        ACPI_MADT_ENTRY *entry =
            (ACPI_MADT_ENTRY*)ptr;

        if(entry->length == 0)
            break;

        if(entry->type == 2)
        {
            ACPI_MADT_ISO *iso =
                (ACPI_MADT_ISO*)entry;

            if(iso->source == irq)
                return iso->gsi;
        }

        ptr += entry->length;
    }

    /*
        No override found.
        Legacy mapping.
    */
    return irq;
}


/*
    --------------------------------------------------
    Get ISO Entry for IRQ
    --------------------------------------------------
*/

ACPI_MADT_ISO*
acpi_get_iso(
    uint8_t irq
)
{
    if(!acpi_madt)
        return 0;

    uint8_t *ptr =
        (uint8_t*)acpi_madt +
        sizeof(ACPI_MADT);

    uint8_t *end =
        (uint8_t*)acpi_madt +
        acpi_madt->header.length;

    while(ptr < end)
    {
        ACPI_MADT_ENTRY *entry =
            (ACPI_MADT_ENTRY*)ptr;

        if(entry->length == 0)
            break;

        if(entry->type == 2)
        {
            ACPI_MADT_ISO *iso =
                (ACPI_MADT_ISO*)entry;

            if(iso->source == irq)
                return iso;
        }

        ptr += entry->length;
    }

    return 0;
}

void acpi_general_print() {
    if(acpi_detect())
    {
        acpi_get_pm_base();

        acpi_dump();

        acpi_enable();

        if(acpi_is_enabled())
            kprintf("[ACPI] Enabled\n");
        
        kprintf("SCI IRQ: %u\n", acpi_get_sci_irq());

        kprintf("PM1 Status: 0x%x\n", acpi_get_pm1_status());

        kprintf("PM1 Control: 0x%x\n", acpi_get_pm1_control());

        kprintf("PM Timer: %u\n", acpi_get_pm_timer());

        acpi_clear_events();

        acpi_find_rsdp();

        acpi_find_rsdt();
        acpi_find_xsdt();

        acpi_find_fadt();
        acpi_find_dsdt();
        acpi_find_madt();


        acpi_dump_rsdp();
        acpi_dump_rsdt();
        acpi_dump_xsdt();

        acpi_dump_fadt();
        acpi_dump_dsdt();
        acpi_dump_madt();

        acpi_parse_madt();
        uint8_t *ptr =
        (uint8_t*)acpi_madt + sizeof(ACPI_MADT);

        while(ptr < ((uint8_t*)acpi_madt + acpi_madt->header.length))
        {
            ACPI_MADT_ENTRY *entry =
                (ACPI_MADT_ENTRY*)ptr;

            if(entry->length == 0)
                break;

            if(entry->type == 0)
            {
                acpi_parse_lapic(
                    (ACPI_MADT_LAPIC*)entry
                );

                break;
            }

            ptr += entry->length;
        }

        acpi_dump_cpus();

        ACPI_MADT_IOAPIC *ioapic =
            acpi_get_first_ioapic();

        if(ioapic)
        {
            acpi_parse_ioapic(ioapic);
        }

        ACPI_MADT_IOAPIC *ioapick =
            acpi_get_first_ioapic();

        if(ioapick)
        {
            acpi_dump_ioapic(ioapick);
        }

        acpi_dump_iso();
    }
    else
    {
        kprintf("[ACPI] Controller not found\n");
    }   
}

void acpi_init(cs_task *self)
{
    (void)self;

    reg_t vol_t unum8_t *slot = (vol_t unum8_t*)KDI;
    reg_t unum8_t debug = (num_t)(*slot);

    // Detect ACPI
    if(!acpi_detect())
    {
        if (debug != 2) { kprintf("[ACPI] ERROR: Controller not found\n"); }
        return;
    }


    // Find ACPI tables
    acpi_find_rsdp();

    if(!acpi_rsdp)
    {
        if (debug != 2) { kprintf("[ACPI] ERROR: RSDP not found\n"); }
        return;
    }

    acpi_find_rsdt();
    acpi_find_xsdt();

    acpi_find_fadt();
    acpi_find_dsdt();
    acpi_find_madt();


    if(!acpi_fadt)
    {
        if (debug != 2) { kprintf("[ACPI] ERROR: FADT not found\n"); }
        return;
    }


    // Power management
    acpi_get_pm_base();

    acpi_enable();

    if(!acpi_is_enabled())
    {
        if (debug != 2) { kprintf("[ACPI] ERROR: Failed to enable ACPI\n"); }
        return;
    }


    // Clear pending events
    acpi_clear_events();
}

void acpi_deinit(cs_task *self)
{
    (void)self;

    // Clear pending events before handing control back
    if(acpi.pm_base)
    {
        acpi_clear_events();
    }


    // Disable ACPI (returns control to SMI)
    if(acpi_is_enabled())
    {
        acpi_disable();
    }


    // Reset table pointers
    acpi_rsdp = 0;
    acpi_rsdt = 0;
    acpi_xsdt = 0;
    acpi_fadt = 0;
    acpi_dsdt = 0;
    acpi_madt = 0;


    // Reset parsed counters
    acpi_cpu_count    = 0;
    acpi_ioapic_count = 0;
    acpi_iso_count    = 0;


    // Reset device state
    acpi.vendor  = 0;
    acpi.device  = 0;
    acpi.bus     = 0;
    acpi.dev     = 0;
    acpi.fun     = 0;
    acpi.pm_base = 0;
    acpi.enabled = 0;
}

/*
    --------------------------------------------------
    ACPI Sleep / Shutdown / Reboot
    CoreSys
    --------------------------------------------------
*/

#define ACPI_SLP_EN         (1 << 13)
#define ACPI_SLP_TYP_SHIFT  10
#define ACPI_SLP_TYP_MASK   (0x7 << ACPI_SLP_TYP_SHIFT)
#define ACPI_RESET_REG_SUP  (1 << 10)

/*
    Locate the \_Sx package inside the DSDT AML and
    extract SLP_TYPa / SLP_TYPb.

    CoreSys has no AML interpreter, so this walks the
    raw AML byte stream for the object name (e.g.
    "_S5_") and decodes the two ByteConst/ByteData
    operands that follow the Package header.
*/
int acpi_find_sx(
    uint8_t state,
    uint8_t *slp_typa,
    uint8_t *slp_typb
)
{
    if(!acpi_dsdt)
        return 0;

    if(state > 5)
        return 0;

    char name[5] = "_S0_";
    name[2] = '0' + state;

    uint8_t *ptr =
        (uint8_t*)acpi_dsdt->aml;

    uint8_t *end =
        (uint8_t*)acpi_dsdt +
        acpi_dsdt->header.length;

    while(ptr < end - 5)
    {
        if(ptr[0] == 0x08 &&
           ptr[1] == (uint8_t)name[0] &&
           ptr[2] == (uint8_t)name[1] &&
           ptr[3] == (uint8_t)name[2] &&
           ptr[4] == (uint8_t)name[3])
        {
            uint8_t *pkg = ptr + 5;

            /*
                PackageOp
            */
            if(*pkg != 0x12)
            {
                ptr++;
                continue;
            }

            pkg++;

            /*
                PkgLength: top two bits of the lead
                byte give the number of additional
                length bytes that follow it.
            */
            uint8_t lead = *pkg;
            uint8_t extra_bytes = (lead >> 6) & 0x3;

            pkg += 1 + extra_bytes;

            /*
                NumElements
            */
            pkg++;

            /*
                Element 0 = SLP_TYPa
            */
            uint8_t v0 = *pkg;

            if(v0 == 0x0A)
            {
                pkg++;
                v0 = *pkg;
            }

            pkg++;

            /*
                Element 1 = SLP_TYPb
            */
            uint8_t v1 = *pkg;

            if(v1 == 0x0A)
            {
                pkg++;
                v1 = *pkg;
            }

            *slp_typa = v0;
            *slp_typb = v1;

            return 1;
        }

        ptr++;
    }

    return 0;
}

/*
    --------------------------------------------------
    Enter an ACPI sleep state (S1-S5)
    --------------------------------------------------
*/
int acpi_sleep(
    uint8_t state
)
{
    if(state < 1 || state > 5)
    {
        return 0;
    }

    if(!acpi_dsdt)
    {
        return 0;
    }

    if(!acpi.pm_base)
    {
        return 0;
    }

    uint8_t slp_typa = 0;
    uint8_t slp_typb = 0;

    if(!acpi_find_sx(state, &slp_typa, &slp_typb))
    {
        return 0;
    }

    uint16_t pm1a =
        inw(acpi.pm_base + 4);

    pm1a &= ~ACPI_SLP_TYP_MASK;
    pm1a |= ((uint16_t)slp_typa << ACPI_SLP_TYP_SHIFT) & ACPI_SLP_TYP_MASK;

    outw(
        acpi.pm_base + 4,
        pm1a | ACPI_SLP_EN
    );

    if(acpi_fadt &&
       acpi_fadt->pm1b_cnt_blk)
    {
        uint16_t pm1b =
            inw((uint16_t)acpi_fadt->pm1b_cnt_blk);

        pm1b &= ~ACPI_SLP_TYP_MASK;
        pm1b |= ((uint16_t)slp_typb << ACPI_SLP_TYP_SHIFT) & ACPI_SLP_TYP_MASK;

        outw(
            (uint16_t)acpi_fadt->pm1b_cnt_blk,
            pm1b | ACPI_SLP_EN
        );
    }

    /*
        For S1-S4 the CPU resumes execution here on
        wake. S5 is a full power-off; if control
        returns at all here, entry failed.
    */

    for(volatile int i = 0; i < 1000000; i++);

    return 0;
}

/*
    --------------------------------------------------
    Shutdown (S5)
    --------------------------------------------------
*/
int acpi_shutdown(void)
{
    acpi_sleep(5);

    return 0;
}

/*
    --------------------------------------------------
    Reboot
    --------------------------------------------------
*/
int acpi_reboot(void)
{
    if(acpi_fadt &&
       (acpi_fadt->flags & ACPI_RESET_REG_SUP) &&
       acpi_fadt->reset_reg)
    {
        outb(
            (uint16_t)acpi_fadt->reset_reg,
            acpi_fadt->reset_value
        );

        for(volatile int i = 0; i < 1000000; i++);
    }

    /*
        ACPI reset unavailable or did not take.
        Fall back to the keyboard controller pulse.
    */

    uint8_t status;

    do
    {
        status = inb(0x64);

        if(status & 0x01)
            inb(0x60);

    } while(status & 0x02);

    outb(0x64, 0xFE);

    for(volatile int i = 0; i < 1000000; i++);

    return 0;
}
