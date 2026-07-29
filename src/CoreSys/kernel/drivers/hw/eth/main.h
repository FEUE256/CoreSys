// PCI/PCIe Ethernet Driver (Approved by FÈUE in the CoreSys kernel hardware drivers collation)
#pragma once

#include <drivers/serial/main.h>
#include <drivers/pci/main.h>
#include <drivers/sf/main.h>
#include <drivers/task/main.h>
#include <drivers/halt/main.h>

#include <kernel/mem.h>

#include <stdint.h>

#define KERNEL_IP 0x0F02000A

#define E1000_BUS  0
#define E1000_DEV  5
#define E1000_FUNC 0

#define E1000_BAR0 0x10

#define IPV4_PROTO_ICMP 1
#define IPV4_PROTO_TCP  6
#define IPV4_PROTO_UDP 17

#define ICMP_ECHO_REPLY    0
#define ICMP_ECHO_REQUEST  8

static uint8_t ping_mac[6];
static uint32_t ping_ip;
static uint8_t ping_arp_done = 0;

typedef struct __attribute__((packed))
{
    uint8_t dst[6];       // Destination MAC
    uint8_t src[6];       // Source MAC
    uint16_t type;        // EtherType

} ethernet_header_t;

void arp_receive(
    ethernet_header_t *eth,
    void *data,
    uint16_t length
);

static uint16_t icmp_checksum(
    void *data,
    uint16_t length
);

int e1000_send(const void *data, uint16_t length);
int e1000_receive(void *buffer, uint16_t *length);

typedef struct __attribute__((packed))
{
    uint8_t version_ihl;
    uint8_t dscp_ecn;

    uint16_t total_length;

    uint16_t identification;

    uint16_t flags_fragment;

    uint8_t ttl;

    uint8_t protocol;

    uint16_t checksum;

    uint32_t src_ip;

    uint32_t dst_ip;

} ipv4_header_t;

void ipv4_receive(
    ethernet_header_t *eth,
    void *data,
    uint16_t length
);
int ipv4_send(
    uint8_t *dst_mac,
    uint32_t dst_ip,
    uint8_t protocol,
    void *payload,
    uint16_t payload_length
);



// Registers

#define E1000_CTRL      0x0000
#define E1000_STATUS    0x0008

#define E1000_EERD      0x0014

#define E1000_ICR       0x00C0
#define E1000_IMS       0x00D0

#define E1000_RCTL      0x0100

#define E1000_TCTL      0x0400


#define E1000_RDBAL     0x2800
#define E1000_RDBAH     0x2804
#define E1000_RDLEN     0x2808
#define E1000_RDH       0x2810
#define E1000_RDT       0x2818


#define E1000_TDBAL     0x3800
#define E1000_TDBAH     0x3804
#define E1000_TDLEN     0x3808
#define E1000_TDH       0x3810
#define E1000_TDT       0x3818

#define E1000_RDBAL 0x2800
#define E1000_RDBAH 0x2804
#define E1000_RDLEN 0x2808
#define E1000_RDH   0x2810
#define E1000_RDT   0x2818
#define E1000_RCTL  0x0100

typedef struct
{
    uint16_t vendor;
    uint16_t device;

    uint64_t mmio;

    uint8_t mac[6];

    uint8_t initialized;

} E1000_DEVICE;


static E1000_DEVICE e1000;



// MMIO read

static uint32_t e1000_read(uint32_t reg)
{
    volatile uint32_t *addr =
        (volatile uint32_t*)
        (e1000.mmio + reg);

    return *addr;
}


// MMIO write

void e1000_write(uint32_t reg,uint32_t value)
{
    volatile uint32_t *addr =
        (volatile uint32_t*)
        (e1000.mmio + reg);

    *addr = value;
}

#define E1000_RX_COUNT 32
#define E1000_RX_BUFFER_SIZE 2048


struct e1000_rx_desc
{
    uint64_t addr;

    uint16_t length;
    uint16_t checksum;

    uint8_t status;
    uint8_t errors;

    uint16_t special;
};


static volatile struct e1000_rx_desc
    e1000_rx_ring[E1000_RX_COUNT]
    __attribute__((aligned(16)));


static uint8_t
    e1000_rx_buffers[E1000_RX_COUNT][E1000_RX_BUFFER_SIZE];


// =====================================
// RX Init
// =====================================

void e1000_rx_init(void)
{
    kprintf("[E1000] RX Init\n");


    /*
        Clear descriptors
    */

    for(uint32_t i = 0;
        i < E1000_RX_COUNT;
        i++)
    {
        e1000_rx_ring[i].addr =
            (uint64_t)(uintptr_t)
            &e1000_rx_buffers[i][0];


        e1000_rx_ring[i].length  = 0;
        e1000_rx_ring[i].checksum = 0;
        e1000_rx_ring[i].status   = 0;
        e1000_rx_ring[i].errors   = 0;
        e1000_rx_ring[i].special  = 0;
    }



    /*
        RX Descriptor Base Low
    */

    uint64_t rx_addr =
    (uint64_t)(uintptr_t)&e1000_rx_ring;

    e1000_write(
        E1000_RDBAL,
        rx_addr & 0xffffffff
    );

    e1000_write(
        E1000_RDBAH,
        rx_addr >> 32
    );


    e1000_write(
        E1000_RDBAL,
        (uint32_t)(rx_addr & 0xFFFFFFFF)
    );

    /*
        RX Descriptor Base High
    */

    e1000_write(
        E1000_RDBAH,
        (uint32_t)
        (((uint64_t)&e1000_rx_ring) >> 32)
    );



    /*
        Ring size
    */

    e1000_write(
        E1000_RDLEN,
        sizeof(e1000_rx_ring)
    );



    /*
        Head / Tail
    */

    e1000_write(
        E1000_RDH,
        0
    );


    e1000_write(
        E1000_RDT,
        E1000_RX_COUNT - 1
    );



    /*
        Enable RX
        - EN
        - Accept broadcast
        - 2048 byte buffers
    */
    uint32_t rctl = 0;

    rctl |= (1 << 1);   // RX enable
    rctl |= (1 << 15);  // Broadcast accept
    rctl |= (1 << 26);  // Strip CRC

    e1000_write(E1000_RCTL, rctl);


    e1000_write(
        E1000_RCTL,
        rctl
    );


    kprintf(
        "[E1000] RX Ready\n"
    );
}

// =====================================
// E1000 TX Initialization
// =====================================

#define E1000_TX_COUNT 32
#define E1000_TX_BUFFER_SIZE 2048


struct e1000_tx_desc
{
    uint64_t addr;

    uint16_t length;

    uint8_t cso;
    uint8_t cmd;

    uint8_t status;
    uint8_t css;

    uint16_t special;
};


static struct e1000_tx_desc
    e1000_tx_ring[E1000_TX_COUNT];


static uint8_t
    e1000_tx_buffers[E1000_TX_COUNT][E1000_TX_BUFFER_SIZE];

#define E1000_TDBAL 0x3800
#define E1000_TDBAH 0x3804
#define E1000_TDLEN 0x3808
#define E1000_TDH   0x3810
#define E1000_TDT   0x3818
#define E1000_TCTL  0x0400
#define E1000_TIPG  0x0410

// =====================================
// TX Init
// =====================================

void e1000_tx_init(void)
{
    kprintf("[E1000] TX Init\n");


    for(uint32_t i = 0;
        i < E1000_TX_COUNT;
        i++)
    {
        e1000_tx_ring[i].addr =
            (uint64_t)(uintptr_t)
            &e1000_tx_buffers[i];


        e1000_tx_ring[i].length =
            0;

        e1000_tx_ring[i].cso =
            0;

        e1000_tx_ring[i].cmd =
            0;

        /*
            Descriptor done
            so hardware can use it
        */
        e1000_tx_ring[i].status =
            0x1;

        e1000_tx_ring[i].css =
            0;

        e1000_tx_ring[i].special =
            0;
    }



    uint64_t tx_addr =
        (uint64_t)(uintptr_t)
        &e1000_tx_ring;



    /*
        TX Descriptor Base Low
    */

    e1000_write(
        E1000_TDBAL,
        (uint32_t)(tx_addr & 0xFFFFFFFF)
    );


    /*
        TX Descriptor Base High
    */

    e1000_write(
        E1000_TDBAH,
        (uint32_t)(tx_addr >> 32)
    );



    /*
        Descriptor ring length
    */

    e1000_write(
        E1000_TDLEN,
        sizeof(e1000_tx_ring)
    );



    /*
        Head / Tail
    */

    e1000_write(
        E1000_TDH,
        0
    );


    e1000_write(
        E1000_TDT,
        0
    );



    /*
        Enable transmitter
    */

    uint32_t tctl =
        0;


    /*
        TCTL.EN
    */
    tctl |= (1 << 1);


    /*
        Collision threshold
    */
    tctl |= (0x10 << 4);


    /*
        Collision distance
    */
    tctl |= (0x40 << 12);



    e1000_write(
        E1000_TCTL,
        tctl
    );



    /*
        Inter packet gap
    */

    e1000_write(
        E1000_TIPG,
        0x0060200A
    );


    kprintf(
        "[E1000] TX Ready\n"
    );
}

// =====================================
// Init
// =====================================

void e1000_init(void)
{
    kprintf("[E1000] Init\n");


    e1000.vendor =
        pci_vendor(
            E1000_BUS,
            E1000_DEV,
            E1000_FUNC
        );


    e1000.device =
        pci_device(
            E1000_BUS,
            E1000_DEV,
            E1000_FUNC
        );


    uint32_t bar =
        pci_read(
            E1000_BUS,
            E1000_DEV,
            E1000_FUNC,
            E1000_BAR0
        );


    e1000.mmio =
        bar & ~0xF;


    if(!e1000.mmio)
    {
        kprintf("[E1000] No MMIO\n");
        return;
    }


    // Enable PCI memory + bus master

    uint32_t cmd =
        pci_read(
            E1000_BUS,
            E1000_DEV,
            E1000_FUNC,
            0x04
        );


    cmd |= (1 << 1);
    cmd |= (1 << 2);


    pci_write(
        E1000_BUS,
        E1000_DEV,
        E1000_FUNC,
        0x04,
        cmd
    );

    e1000_rx_init();
    e1000_tx_init();

    e1000.initialized = 1;


    kprintf("[E1000] MMIO: 0x%x\n",
        (uint32_t)e1000.mmio);
}

#define E1000_RAL0 0x5400
#define E1000_RAH0 0x5404


void e1000_read_mac(void)
{
    uint32_t ral =
        e1000_read(E1000_RAL0);

    uint32_t rah =
        e1000_read(E1000_RAH0);


    e1000.mac[0] = ral & 0xff;
    e1000.mac[1] = (ral >> 8) & 0xff;
    e1000.mac[2] = (ral >> 16) & 0xff;
    e1000.mac[3] = (ral >> 24) & 0xff;

    e1000.mac[4] = rah & 0xff;
    e1000.mac[5] = (rah >> 8) & 0xff;
}

// =====================================
// PCI dump
// =====================================

void e1000_dump_pci(void)
{
    kprintf("[E1000] PCI\n");

    kprintf("Vendor: 0x%x\n",
        e1000.vendor);

    kprintf("Device: 0x%x\n",
        e1000.device);
}



// =====================================
// Registers
// =====================================

void e1000_dump_registers(void)
{
    kprintf("[E1000] Registers\n");


    kprintf(
        "CTRL: 0x%x\n",
        e1000_read(E1000_CTRL)
    );


    kprintf(
        "STATUS: 0x%x\n",
        e1000_read(E1000_STATUS)
    );


    kprintf(
        "RCTL: 0x%x\n",
        e1000_read(E1000_RCTL)
    );


    kprintf(
        "TCTL: 0x%x\n",
        e1000_read(E1000_TCTL)
    );
}



// =====================================
// MAC
// =====================================

void e1000_dump_mac(void)
{
    kprintf("[E1000] MAC\n");

    e1000_read_mac();
    
    kprintf(
        "%x:%x:%x:%x:%x:%x\n",
        e1000.mac[0],
        e1000.mac[1],
        e1000.mac[2],
        e1000.mac[3],
        e1000.mac[4],
        e1000.mac[5]
    );
}



// =====================================
// Link
// =====================================

void e1000_dump_link(void)
{
    uint32_t status =
        e1000_read(E1000_STATUS);


    kprintf("[E1000] Link\n");


    if(status & (1 << 1))
        kprintf("LINK UP\n");
    else
        kprintf("LINK DOWN\n");
}

// =====================================
// ARP Protocol
// =====================================

#define ARP_HTYPE_ETHERNET 0x0001
#define ARP_PTYPE_IPV4     0x0800

#define ARP_REQUEST 1
#define ARP_REPLY   2


typedef struct __attribute__((packed))
{
    uint16_t htype;      // Hardware type
    uint16_t ptype;      // Protocol type

    uint8_t hlen;        // Hardware address length
    uint8_t plen;        // Protocol address length

    uint16_t oper;       // Operation

    uint8_t sha[6];      // Sender MAC
    uint32_t spa;        // Sender IP

    uint8_t tha[6];      // Target MAC
    uint32_t tpa;        // Target IP

} arp_packet_t;



static uint16_t arp_ntohs(uint16_t value)
{
    return (value >> 8) | (value << 8);
}



// Print IPv4

void arp_print_ip(uint32_t ip)
{
    kprintf(
        "%u.%u.%u.%u",
        (ip >> 0) & 0xff,
        (ip >> 8) & 0xff,
        (ip >> 16) & 0xff,
        (ip >> 24) & 0xff
    );
}



// Print MAC

void arp_print_mac(uint8_t *mac)
{
    kprintf(
        "%x:%x:%x:%x:%x:%x",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );
}

// =====================================
// ARP Send Reply
// =====================================

void arp_send_reply(
    uint8_t *target_mac,
    uint32_t target_ip
)
{
    uint8_t packet[64] = {0};


    /*
        Ethernet Header
    */

    // Destination MAC
    memcpy(
        packet,
        target_mac,
        6
    );


    // Source MAC
    memcpy(
        packet + 6,
        e1000.mac,
        6
    );


    // EtherType ARP
    packet[12] = 0x08;
    packet[13] = 0x06;



    /*
        ARP Header starts at byte 14
    */

    arp_packet_t *arp =
        (arp_packet_t*)
        (packet + 14);

    arp->hlen = 6;
    arp->plen = 4;


    arp->htype = arp_ntohs(ARP_HTYPE_ETHERNET);
    arp->ptype = arp_ntohs(ARP_PTYPE_IPV4);
    arp->oper  = arp_ntohs(ARP_REPLY);



    /*
        Sender
    */

    memcpy(
        arp->sha,
        e1000.mac,
        6
    );


    /*
        Your IP
        Change this to your kernel IP
    */

    arp->spa =
        KERNEL_IP;   // 10.0.2.15



    /*
        Target
    */

    memcpy(
        arp->tha,
        target_mac,
        6
    );


    arp->tpa =
        target_ip;



    kprintf(
        "[ARP] Sending reply\n"
    );


    e1000_send(
        packet,
        42
    );
}

typedef struct __attribute__((packed))
{
    uint8_t  type;
    uint8_t  code;

    uint16_t checksum;

    uint16_t identifier;
    uint16_t sequence;

} icmp_header_t;

void icmp_send_echo_reply(
    ethernet_header_t *eth,
    ipv4_header_t *ip,
    icmp_header_t *icmp,
    uint16_t length
);

// =====================================
// ARP Receive
// =====================================

uint8_t host_mac[6];

// =====================================
// Ethernet Layer
// =====================================

#define ETH_TYPE_IPV4 0x0800
#define ETH_TYPE_ARP  0x0806

// Convert big endian -> host endian

uint16_t ethernet_ntohs(uint16_t value)
{
    return (value >> 8) | (value << 8);
}



// Print MAC address

void ethernet_print_mac(uint8_t *mac)
{
    kprintf(
        "%x:%x:%x:%x:%x:%x",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );
}



// =====================================
// Ethernet Receive Parser
// =====================================
void ethernet_receive(
    void *frame,
    uint16_t length
)
{
    if(length < sizeof(ethernet_header_t))
    {
        kprintf(
            "[ETH] Invalid frame\n"
        );
        return;
    }


    ethernet_header_t *eth =
        (ethernet_header_t*)frame;



    uint16_t type =
        ethernet_ntohs(
            eth->type
        );


    kprintf(
        "[ETH] Frame\n"
    );


    kprintf(
        "SRC: "
    );

    ethernet_print_mac(
        eth->src
    );

    kprintf("\n");


    kprintf(
        "DST: "
    );

    ethernet_print_mac(
        eth->dst
    );

    kprintf("\n");


    kprintf(
        "TYPE: 0x%x\n",
        type
    );



    switch(type)
    {

        case ETH_TYPE_ARP:

            kprintf(
                "[ETH] ARP packet\n"
            );

            arp_receive(
                eth,
                (uint8_t*)frame + sizeof(ethernet_header_t),
                length - sizeof(ethernet_header_t)
            );

            break;



        case ETH_TYPE_IPV4:

            kprintf(
                "[ETH] IPv4 packet\n"
            );

            ipv4_receive(
                eth,
                (uint8_t*)frame + sizeof(ethernet_header_t),
                length - sizeof(ethernet_header_t)
            );

            break;



        default:

            kprintf(
                "[ETH] Unknown EtherType\n"
            );

            break;
    }
}

// =====================================
// Ethernet Send
// =====================================

int ethernet_send(
    uint8_t *dst_mac,
    uint16_t type,
    void *payload,
    uint16_t payload_length
)
{
    uint8_t frame[2048];


    if(payload_length + sizeof(ethernet_header_t) > sizeof(frame))
    {
        kprintf("[ETH] Frame too large\n");
        return -1;
    }



    ethernet_header_t *eth =
        (ethernet_header_t*)frame;



    /*
        Destination MAC
    */

    memcpy(
        eth->dst,
        dst_mac,
        6
    );


    /*
        Source MAC
    */

    memcpy(
        eth->src,
        e1000.mac,
        6
    );


    /*
        EtherType
        Network byte order
    */

    eth->type = (uint16_t)(
        (type >> 8) |
        (type << 8)
    );



    /*
        Copy payload
    */

    memcpy(
        frame + sizeof(ethernet_header_t),
        payload,
        payload_length
    );



    uint16_t frame_length =
        sizeof(ethernet_header_t)
        +
        payload_length;



    kprintf(
        "[ETH] TX type=0x%x length=%u\n",
        type,
        frame_length
    );


    return e1000_send(
        frame,
        frame_length
    );
}

void e1000_poll(void)
{
    uint8_t buffer[2048];
    uint16_t length;

    if(e1000_receive(buffer,&length))
    {
        kprintf("[RX] Got %u bytes\n", length);

        ethernet_receive(buffer,length);
    }
}

// =====================================
// RX Dump
// =====================================

void e1000_dump_rx(void)
{
    kprintf("[E1000] RX\n");


    kprintf(
        "RDBAL: 0x%x\n",
        e1000_read(E1000_RDBAL)
    );


    kprintf(
        "RDBAH: 0x%x\n",
        e1000_read(E1000_RDBAH)
    );


    kprintf(
        "RDLEN: 0x%x\n",
        e1000_read(E1000_RDLEN)
    );


    kprintf(
        "RDH: 0x%x\n",
        e1000_read(E1000_RDH)
    );


    kprintf(
        "RDT: 0x%x\n",
        e1000_read(E1000_RDT)
    );


    kprintf(
        "RCTL: 0x%x\n",
        e1000_read(E1000_RCTL)
    );


    kprintf(
        "RX Enabled: %s\n",
        (e1000_read(E1000_RCTL) & 0x2)
        ? "YES"
        : "NO"
    );
}

void icmp_send_echo_reply(
    ethernet_header_t *eth,
    ipv4_header_t *ip,
    icmp_header_t *request,
    uint16_t length
)
{
    uint8_t packet[128];

    if (length < sizeof(icmp_header_t))
        return;

    uint16_t payload_len = length - sizeof(icmp_header_t);
    uint16_t max_payload = sizeof(packet) - sizeof(icmp_header_t);

    if (payload_len > max_payload)
        payload_len = max_payload;

    icmp_header_t *reply = (icmp_header_t*)packet;

    memcpy(
        packet + sizeof(icmp_header_t),
        (uint8_t*)request + sizeof(icmp_header_t),
        payload_len
    );

    reply->type = ICMP_ECHO_REPLY;
    reply->code = 0;
    reply->identifier = request->identifier;
    reply->sequence   = request->sequence;
    reply->checksum   = 0;

    uint16_t reply_len = sizeof(icmp_header_t) + payload_len;

    reply->checksum = icmp_checksum(packet, reply_len);

    ipv4_send(eth->src, ip->src_ip, IPV4_PROTO_ICMP, packet, reply_len);
}

// =====================================
// TX
// =====================================

void e1000_dump_tx(void)
{
    kprintf("[E1000] TX\n");


    kprintf(
        "TDBAL: 0x%x\n",
        e1000_read(E1000_TDBAL)
    );

    kprintf(
        "TDLEN: 0x%x\n",
        e1000_read(E1000_TDLEN)
    );
}

// TX descriptor command bits
#define E1000_TX_CMD_EOP  (1 << 0)
#define E1000_TX_CMD_IFCS (1 << 1)
#define E1000_TX_CMD_RS   (1 << 3)

// TX descriptor status bits
#define E1000_TX_STATUS_DD 0x1

int e1000_send(const void *data, uint16_t length)
{
    uint32_t tail =
        e1000_read(E1000_TDT);

    struct e1000_tx_desc *desc =
        &e1000_tx_ring[tail];

    if(!(desc->status & E1000_TX_STATUS_DD))
        return -1;

    memcpy(
        e1000_tx_buffers[tail],
        data,
        length
    );

    desc->length = length;
    desc->cmd =
        E1000_TX_CMD_EOP |
        E1000_TX_CMD_IFCS |
        E1000_TX_CMD_RS;

    desc->status = 0;

    tail++;
    tail %= E1000_TX_COUNT;

    e1000_write(
        E1000_TDT,
        tail
    );

    return 0;
}

int e1000_receive(
    void *buffer,
    uint16_t *length
)
{
    static uint32_t index = 0;


    volatile struct e1000_rx_desc *desc =
        &e1000_rx_ring[index];


    if(!(desc->status & 0x01))
        return 0;


    memcpy(
        buffer,
        e1000_rx_buffers[index],
        desc->length
    );


    *length = desc->length;


    desc->status = 0;


    e1000_write(
        E1000_RDT,
        (index + E1000_RX_COUNT - 1) % E1000_RX_COUNT
    );


    index++;

    if(index >= E1000_RX_COUNT)
        index = 0;


    return 1;
}

static uint16_t ipv4_ntohs(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

void ipv4_print_ip(uint32_t ip)
{
    kprintf(
        "%u.%u.%u.%u",
        ip & 0xff,
        (ip >> 8) & 0xff,
        (ip >> 16) & 0xff,
        (ip >> 24) & 0xff
    );
}

// Internet checksum
static uint16_t icmp_checksum(void *data, uint16_t length)
{
    uint32_t sum = 0;

    uint16_t *ptr = (uint16_t*)data;

    while(length > 1)
    {
        sum += *ptr++;
        length -= 2;
    }

    if(length)
        sum += *(uint8_t*)ptr;

    while(sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)(~sum);
}


void icmp_receive(
    ethernet_header_t *eth,
    ipv4_header_t *ip,
    void *data,
    uint16_t length
)
{
    if(length < sizeof(icmp_header_t))
    {
        kprintf("[ICMP] Packet too small\n");
        return;
    }

    icmp_header_t *icmp =
        (icmp_header_t*)data;

    kprintf("[ICMP]\n");

    kprintf(
        "Type: %u\n",
        icmp->type
    );

    kprintf(
        "Code: %u\n",
        icmp->code
    );

    kprintf(
        "Identifier: %u\n",
        arp_ntohs(icmp->identifier)
    );

    kprintf(
        "Sequence: %u\n",
        arp_ntohs(icmp->sequence)
    );

    switch(icmp->type)
    {
        case ICMP_ECHO_REQUEST:

            kprintf(
                "[ICMP] Echo Request\n"
            );

            icmp_send_echo_reply(
                eth,
                ip,
                icmp,
                length
            );
            break;

        case ICMP_ECHO_REPLY:

            kprintf(
                "[ICMP] Echo Reply\n"
            );

            break;

        default:

            kprintf(
                "[ICMP] Unknown Type\n"
            );

            break;
    }
}

#define UDP_PROTOCOL 17

typedef struct __attribute__((packed))
{
    uint16_t src_port;
    uint16_t dst_port;

    uint16_t length;
    uint16_t checksum;

} udp_header_t;


static uint16_t udp_ntohs(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

// DHCP Protocol

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68


#define DHCP_BOOTREQUEST 1
#define DHCP_BOOTREPLY   2


#define DHCP_DISCOVER 1
#define DHCP_OFFER    2
#define DHCP_REQUEST  3
#define DHCP_ACK      5



typedef struct __attribute__((packed))
{
    uint8_t op;

    uint8_t htype;

    uint8_t hlen;

    uint8_t hops;


    uint32_t xid;


    uint16_t secs;

    uint16_t flags;


    uint32_t ciaddr;

    uint32_t yiaddr;

    uint32_t siaddr;

    uint32_t giaddr;


    uint8_t chaddr[16];


    uint8_t sname[64];

    uint8_t file[128];


    uint32_t magic;


    uint8_t options[];

} dhcp_packet_t;



void dhcp_print_ip(uint32_t ip)
{
    kprintf(
        "%u.%u.%u.%u",
        (ip >> 0) & 0xff,
        (ip >> 8) & 0xff,
        (ip >> 16) & 0xff,
        (ip >> 24) & 0xff
    );
}



uint32_t dhcp_ip;



// =====================================
// DHCP Receive
// =====================================

void dhcp_receive(
    void *data,
    uint16_t length
)
{

    if(length < sizeof(dhcp_packet_t))
    {
        kprintf("[DHCP] Invalid packet\n");
        return;
    }


    dhcp_packet_t *dhcp =
        (dhcp_packet_t*)data;



    kprintf(
        "[DHCP] Packet\n"
    );


    kprintf(
        "OP: %u\n",
        dhcp->op
    );


    kprintf(
        "XID: 0x%x\n",
        dhcp->xid
    );


    kprintf(
        "Your IP: "
    );


    dhcp_print_ip(
        dhcp->yiaddr
    );


    kprintf("\n");



    /*
        DHCP options
    */


    uint8_t *opt =
        dhcp->options;



    while(opt < ((uint8_t*)dhcp + length))
    {

        uint8_t type = opt[0];


        if(type == 0xff)
            break;


        if(type == 0)
        {
            opt++;
            continue;
        }


        uint8_t size =
            opt[1];



        if(type == 53)
        {
            // DHCP message type

            uint8_t msg =
                opt[2];


            switch(msg)
            {

                case DHCP_OFFER:

                    kprintf(
                        "[DHCP] OFFER received\n"
                    );

                    break;


                case DHCP_ACK:

                    kprintf(
                        "[DHCP] ACK received\n"
                    );


                    dhcp_ip =
                        dhcp->yiaddr;


                    kprintf(
                        "[DHCP] IP assigned: "
                    );

                    dhcp_print_ip(
                        dhcp_ip
                    );

                    kprintf("\n");


                    break;


                default:

                    kprintf(
                        "[DHCP] Message %u\n",
                        msg
                    );

                    break;
            }
        }


        opt += 2 + size;
    }

}

// =====================================
// DNS Protocol
// =====================================

#define DNS_PORT 53

#define DNS_TYPE_A     1
#define DNS_CLASS_IN   1


typedef struct __attribute__((packed))
{
    uint16_t id;

    uint16_t flags;

    uint16_t questions;
    uint16_t answers;

    uint16_t authority;
    uint16_t additional;

} dns_header_t;



typedef struct __attribute__((packed))
{
    uint16_t type;
    uint16_t class;

    uint32_t ttl;

    uint16_t length;

} dns_answer_t;



static uint16_t dns_ntohs(uint16_t value)
{
    return (value >> 8) | (value << 8);
}


uint32_t dns_ntohl(uint32_t value)
{
    return ((value >> 24) & 0xff) |
           ((value >> 8)  & 0xff00) |
           ((value << 8)  & 0xff0000) |
           ((value << 24) & 0xff000000);
}



// =====================================
// DNS Receive
// =====================================

void dns_receive(void *data, uint16_t length)
{
    if (length < sizeof(dns_header_t))
    {
        kprintf("[DNS] Invalid packet\n");
        return;
    }

    dns_header_t *dns = (dns_header_t*)data;

    uint8_t *begin = (uint8_t*)data;
    uint8_t *end   = begin + length;

    uint16_t flags   = dns_ntohs(dns->flags);
    uint16_t answers = dns_ntohs(dns->answers);

    if (!(flags & 0x8000))
    {
        kprintf("[DNS] Not a response\n");
        return;
    }

    if (answers == 0)
    {
        kprintf("[DNS] No answers\n");
        return;
    }

    uint8_t *ptr = begin + sizeof(dns_header_t);

    if (ptr >= end)
        return;

    /* Skip question name, bounded */
    while (ptr < end && *ptr)
    {
        uint8_t label_len = *ptr;

        if (ptr + 1 + label_len >= end)
            return; /* malformed / truncated name */

        ptr += (size_t)label_len + 1;
    }

    if (ptr >= end)
        return;

    ptr++; /* skip terminating zero */

    if (ptr + 4 > end) /* TYPE + CLASS */
        return;

    ptr += 4;

    if (ptr + sizeof(dns_answer_t) > end)
        return;

    dns_answer_t *answer = (dns_answer_t*)ptr;

    uint16_t type = dns_ntohs(answer->type);
    uint16_t size = dns_ntohs(answer->length);

    ptr += sizeof(dns_answer_t);

    if (type == DNS_TYPE_A && size == 4 && ptr + 4 <= end)
    {
        uint32_t ip = *(uint32_t*)ptr;

        kprintf("[DNS] IPv4: %u.%u.%u.%u\n",
            (ip >> 0) & 0xff, (ip >> 8) & 0xff,
            (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    }
    else
    {
        kprintf("[DNS] Unsupported record %u\n", type);
    }
}

#define NTP_PORT 123


typedef struct __attribute__((packed))
{
    uint8_t li_vn_mode;

    uint8_t stratum;

    uint8_t poll;

    int8_t precision;


    uint32_t root_delay;

    uint32_t root_dispersion;


    uint32_t reference_id;


    uint32_t reference_timestamp_seconds;
    uint32_t reference_timestamp_fraction;


    uint32_t originate_timestamp_seconds;
    uint32_t originate_timestamp_fraction;


    uint32_t receive_timestamp_seconds;
    uint32_t receive_timestamp_fraction;


    uint32_t transmit_timestamp_seconds;
    uint32_t transmit_timestamp_fraction;

} ntp_packet_t;



static uint32_t ntp_ntohl(uint32_t value)
{
    return ((value >> 24) & 0xff) |
           ((value >> 8)  & 0xff00) |
           ((value << 8)  & 0xff0000) |
           ((value << 24) & 0xff000000);
}



// NTP epoch starts 1900
// Unix epoch starts 1970
#define NTP_UNIX_OFFSET 2208988800UL



uint64_t ntp_time;



void ntp_receive(
    void *data,
    uint16_t length
)
{
    if(length < sizeof(ntp_packet_t))
    {
        kprintf(
            "[NTP] Invalid packet\n"
        );

        return;
    }


    ntp_packet_t *ntp =
        (ntp_packet_t*)data;



    uint8_t mode =
        ntp->li_vn_mode & 0x07;


    uint8_t version =
        (ntp->li_vn_mode >> 3) & 0x07;



    kprintf(
        "[NTP] Packet\n"
    );


    kprintf(
        "Version: %u\n",
        version
    );


    kprintf(
        "Mode: %u\n",
        mode
    );


    kprintf(
        "Stratum: %u\n",
        ntp->stratum
    );



    uint32_t seconds =
        ntp_ntohl(
            ntp->transmit_timestamp_seconds
        );



    if(seconds < NTP_UNIX_OFFSET)
    {
        kprintf(
            "[NTP] Invalid timestamp\n"
        );

        return;
    }



    uint32_t unix_time =
        seconds - NTP_UNIX_OFFSET;



    ntp_time = unix_time;



    kprintf(
        "[NTP] Unix time: %u\n",
        unix_time
    );
}

void udp_receive(
    ethernet_header_t *eth,
    ipv4_header_t *ip,
    void *data,
    uint16_t length
)
{
    (void)eth;
    (void)ip;
    if(length < sizeof(udp_header_t))
    {
        kprintf("[UDP] Invalid packet\n");
        return;
    }

    udp_header_t *udp =
        (udp_header_t*)data;

    uint16_t src_port =
        udp_ntohs(udp->src_port);

    uint16_t dst_port =
        udp_ntohs(udp->dst_port);

    uint16_t udp_length =
        udp_ntohs(udp->length);

    kprintf("[UDP] Packet\n");

    kprintf(
        "Source Port      : %u\n",
        src_port
    );

    kprintf(
        "Destination Port : %u\n",
        dst_port
    );

    kprintf(
        "Length           : %u\n",
        udp_length
    );

    kprintf(
        "Checksum         : 0x%x\n",
        udp_ntohs(udp->checksum)
    );

    uint8_t *payload =
        (uint8_t*)udp +
        sizeof(udp_header_t);

    uint16_t payload_length =
        udp_length -
        sizeof(udp_header_t);

    kprintf(
        "Payload Length   : %u\n",
        payload_length
    );

    /*
        Dispatch by destination port
    */

    switch(dst_port)
    {
        case 67:
            kprintf("[UDP] DHCP Server\n");
            dhcp_receive(payload, payload_length);
            break;

        case 68:
            kprintf("[UDP] DHCP Client\n");
            dhcp_receive(payload, payload_length);
            break;

        case 53:
            kprintf("[UDP] DNS\n");
            dns_receive(payload, payload_length);
            break;

        case 123:
            kprintf("[UDP] NTP\n");
            ntp_receive(payload, payload_length);
            break;

        default:
            kprintf(
                "[UDP] Unknown UDP port %u\n",
                dst_port
            );
            break;
    }
}

#define TCP_PROTO 6


#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20



typedef struct __attribute__((packed))
{
    uint16_t src_port;
    uint16_t dst_port;


    uint32_t seq;

    uint32_t ack;


    uint8_t data_offset;

    uint8_t flags;


    uint16_t window;

    uint16_t checksum;

    uint16_t urgent;

} tcp_header_t;



static uint16_t tcp_ntohs(uint16_t value)
{
    return (value >> 8) | (value << 8);
}


static uint32_t tcp_ntohl(uint32_t value)
{
    return ((value >> 24) & 0xff) |
           ((value >> 8) & 0xff00) |
           ((value << 8) & 0xff0000) |
           ((value << 24) & 0xff000000);
}

#define HTTP_BUFFER_SIZE 8192


static char http_buffer[HTTP_BUFFER_SIZE];

static uint32_t http_length = 0;



// =====================================
// HTTP Receive
// =====================================

void http_receive(
    void *data,
    uint16_t length
)
{
    if(length == 0)
    {
        return;
    }


    kprintf(
        "[HTTP] Received %u bytes\n",
        length
    );


    /*
        Store response
    */

    if(http_length + length >= HTTP_BUFFER_SIZE)
    {
        kprintf(
            "[HTTP] Buffer full\n"
        );

        return;
    }


    memcpy(
        http_buffer + http_length,
        data,
        length
    );


    http_length += length;


    http_buffer[http_length] = 0;



    /*
        Check HTTP status
    */

    if(http_length >= 12)
    {
        if(
            http_buffer[0] == 'H' &&
            http_buffer[1] == 'T' &&
            http_buffer[2] == 'T' &&
            http_buffer[3] == 'P'
        )
        {
            kprintf(
                "[HTTP] Response\n"
            );


            kprintf(
                "%s\n",
                http_buffer
            );
        }
    }



    /*
        Find body
        Header ends with:

        \r\n\r\n
    */


    char *body =
        0;


    for(uint32_t i = 0;
        i + 3 < http_length;
        i++)
    {
        if(
            http_buffer[i] == '\r' &&
            http_buffer[i+1] == '\n' &&
            http_buffer[i+2] == '\r' &&
            http_buffer[i+3] == '\n'
        )
        {
            body =
                &http_buffer[i+4];

            break;
        }
    }



    if(body)
    {
        kprintf(
            "[HTTP] Body:\n"
        );

        kprintf(
            "%s\n",
            body
        );
    }
}

// =====================================
// TCP Receive
// =====================================

void tcp_receive(
    ethernet_header_t *eth,
    ipv4_header_t *ip,
    void *data,
    uint16_t length
)
{
    (void)eth;
    (void)ip;

    if(length < sizeof(tcp_header_t))
    {
        kprintf("[TCP] Invalid packet\n");
        return;
    }



    tcp_header_t *tcp =
        (tcp_header_t*)data;



    uint16_t src =
        tcp_ntohs(
            tcp->src_port
        );


    uint16_t dst =
        tcp_ntohs(
            tcp->dst_port
        );



    uint32_t seq =
        tcp_ntohl(
            tcp->seq
        );


    uint32_t ack =
        tcp_ntohl(
            tcp->ack
        );



    uint8_t header_size =
        (tcp->data_offset >> 4) * 4;



    uint8_t flags =
        tcp->flags;



    kprintf("[TCP]\n");


    kprintf(
        "SRC PORT: %u\n",
        src
    );


    kprintf(
        "DST PORT: %u\n",
        dst
    );


    kprintf(
        "SEQ: %u\n",
        seq
    );


    kprintf(
        "ACK: %u\n",
        ack
    );


    kprintf(
        "WINDOW: %u\n",
        tcp_ntohs(tcp->window)
    );



    if(flags & TCP_SYN)
    {
        kprintf(
            "[TCP] SYN\n"
        );
    }


    if(flags & TCP_ACK)
    {
        kprintf(
            "[TCP] ACK\n"
        );
    }


    if(flags & TCP_FIN)
    {
        kprintf(
            "[TCP] FIN\n"
        );
    }


    if(flags & TCP_RST)
    {
        kprintf(
            "[TCP] RST\n"
        );
    }



    /*
        TCP payload
    */


    if(length > header_size)
    {
        uint8_t *payload =
            (uint8_t*)data + header_size;


        uint16_t payload_length =
            length - header_size;


        kprintf(
            "[TCP] Payload %u bytes\n",
            payload_length
        );


        /*
            HTTP data will arrive here
        */

        http_receive(
            payload,
            payload_length
        );
    }
}

void ipv4_receive(
    ethernet_header_t *eth,
    void *data,
    uint16_t length
)
{
    if (length < sizeof(ipv4_header_t))
        return;

    ipv4_header_t *ip = (ipv4_header_t*)data;

    uint8_t version = ip->version_ihl >> 4;
    uint8_t ihl      = ip->version_ihl & 0x0F;

    if (version != 4)
        return;

    if (ihl < 5)
        return;

    uint16_t header_bytes = (uint16_t)ihl * 4;

    if (header_bytes > length)
        return;

    uint16_t total = ipv4_ntohs(ip->total_length);

    /* Never trust the wire value beyond what we actually received. */
    if (total < header_bytes || total > length)
        total = length;

    if (ip->dst_ip != KERNEL_IP)
        return;

    void *payload = (uint8_t*)data + header_bytes;
    uint16_t payload_length = total - header_bytes;

    switch (ip->protocol)
    {
        case IPV4_PROTO_ICMP:
            icmp_receive(eth, ip, payload, payload_length);
            break;

        case IPV4_PROTO_UDP:
            udp_receive(eth, ip, payload, payload_length);
            break;

        case IPV4_PROTO_TCP:
            tcp_receive(eth, ip, payload, payload_length);
            break;
    }
}

// =====================================
// IPv4 Send
// =====================================

static uint16_t ipv4_checksum(
    void *data,
    uint16_t length
)
{
    uint32_t sum = 0;

    uint16_t *ptr = data;

    while(length > 1)
    {
        sum += *ptr++;
        length -= 2;
    }

    if(length)
        sum += *(uint8_t*)ptr;


    while(sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);


    return (uint16_t)(~sum);
}


// =====================================
// IPv4 Send
// =====================================

int ipv4_send(
    uint8_t *dst_mac,
    uint32_t dst_ip,
    uint8_t protocol,
    void *payload,
    uint16_t payload_length
)
{
    uint8_t packet[2048];


    /*
        Ethernet Header
    */

    ethernet_header_t *eth =
        (ethernet_header_t*)packet;


    memcpy(
        eth->dst,
        dst_mac,
        6
    );


    memcpy(
        eth->src,
        e1000.mac,
        6
    );


    eth->type =
        ((ETH_TYPE_IPV4 & 0xff) << 8) |
        ((ETH_TYPE_IPV4 >> 8) & 0xff);



    /*
        IPv4 Header
    */

    ipv4_header_t *ip =
        (ipv4_header_t*)
        (packet + sizeof(ethernet_header_t));


    ip->version_ihl =
        (4 << 4) | 5;


    ip->dscp_ecn = 0;


    ip->total_length =
        ipv4_ntohs(
            sizeof(ipv4_header_t) +
            payload_length
        );


    static uint16_t id = 0;

    ip->identification =
        ipv4_ntohs(id++);


    ip->flags_fragment = 0;


    ip->ttl = 64;


    ip->protocol =
        protocol;


    ip->checksum = 0;


    ip->src_ip =
        KERNEL_IP;


    ip->dst_ip =
        dst_ip;



    ip->checksum =
        ipv4_checksum(
            ip,
            sizeof(ipv4_header_t)
        );



    /*
        Payload
    */

    memcpy(
        packet +
        sizeof(ethernet_header_t) +
        sizeof(ipv4_header_t),

        payload,

        payload_length
    );



    uint16_t total_length =
        sizeof(ethernet_header_t) +
        sizeof(ipv4_header_t) +
        payload_length;



    kprintf(
        "[IPv4] Sending packet %u bytes\n",
        total_length
    );


    return e1000_send(
        packet,
        total_length
    );
}

void arp_request(uint32_t target_ip)
{
    uint8_t packet[64] = {0};

    uint8_t broadcast[6] =
    {
        0xff,0xff,0xff,0xff,0xff,0xff
    };


    memcpy(packet, broadcast, 6);

    memcpy(
        packet + 6,
        e1000.mac,
        6
    );


    packet[12] = 0x08;
    packet[13] = 0x06;


    arp_packet_t *arp =
        (arp_packet_t*)(packet + 14);


    arp->htype =
        arp_ntohs(ARP_HTYPE_ETHERNET);

    arp->ptype =
        arp_ntohs(ARP_PTYPE_IPV4);


    arp->hlen = 6;
    arp->plen = 4;


    arp->oper =
        arp_ntohs(ARP_REQUEST);


    memcpy(
        arp->sha,
        e1000.mac,
        6
    );


    arp->spa =
        KERNEL_IP;


    memset(
        arp->tha,
        0,
        6
    );


    arp->tpa =
        target_ip;


    kprintf("[ARP] Request ");

    arp_print_ip(target_ip);

    kprintf("\n");


    e1000_send(
        packet,
        42
    );
}

// =====================================
// ICMP Ping
// =====================================

int e1000_ping(
    uint8_t *dst_mac,
    uint32_t dst_ip
)
{
    uint8_t packet[64] = {0};


    icmp_header_t *icmp =
        (icmp_header_t*)packet;



    icmp->type =
        ICMP_ECHO_REQUEST;


    icmp->code = 0;


    icmp->identifier =
        arp_ntohs(0x1234);


    static uint16_t seq = 0;


    icmp->sequence =
        arp_ntohs(seq++);



    packet[
        sizeof(icmp_header_t)+0
    ]='P';


    packet[
        sizeof(icmp_header_t)+1
    ]='I';


    packet[
        sizeof(icmp_header_t)+2
    ]='N';


    packet[
        sizeof(icmp_header_t)+3
    ]='G';



    uint16_t size =
        sizeof(icmp_header_t)+4;



    icmp->checksum=0;


    icmp->checksum =
        icmp_checksum(
            packet,
            size
        );



    kprintf(
        "[PING] Echo request\n"
    );



    return ipv4_send(
        dst_mac,
        dst_ip,
        IPV4_PROTO_ICMP,
        packet,
        size
    );
}

void e1000_ping_host(
    uint32_t ip
)
{
    ping_ip = ip;

    ping_arp_done = 0;


    arp_request(ip);



    for(uint32_t i = 0;
        i < 1000000;
        i++)
    {
        e1000_poll();


        if(ping_arp_done)
            break;
    }



    if(!ping_arp_done)
    {
        kprintf(
            "[PING] ARP timeout\n"
        );

        return;
    }



    e1000_ping(
        ping_mac,
        ping_ip
    );
}

void arp_receive(
    ethernet_header_t *eth,
    void *data,
    uint16_t length
)
{
    (void)eth;


    if(length < sizeof(arp_packet_t))
    {
        kprintf("[ARP] Packet too small\n");
        return;
    }


    arp_packet_t *arp =
        (arp_packet_t *)data;



    /*
        Check Ethernet + IPv4 ARP
    */

    if(arp->htype != arp_ntohs(ARP_HTYPE_ETHERNET))
    {
        kprintf("[ARP] Invalid hardware type\n");
        return;
    }


    if(arp->ptype != arp_ntohs(ARP_PTYPE_IPV4))
    {
        kprintf("[ARP] Invalid protocol type\n");
        return;
    }


    if(arp->hlen != 6 ||
       arp->plen != 4)
    {
        kprintf("[ARP] Invalid address size\n");
        return;
    }



    kprintf("[ARP] Packet\n");


    kprintf("Sender MAC: ");
    arp_print_mac(arp->sha);
    kprintf("\n");


    kprintf("Sender IP: ");
    arp_print_ip(arp->spa);
    kprintf("\n");


    kprintf("Target IP: ");
    arp_print_ip(arp->tpa);
    kprintf("\n");



    /*
        ARP request
    */

    if(arp->oper == arp_ntohs(ARP_REQUEST))
    {
        kprintf("[ARP] Request\n");


        /*
            Is the request for CoreSys?
        */

        if(arp->tpa != KERNEL_IP)
        {
            return;
        }


        kprintf("[ARP] Sending reply\n");


        arp_send_reply(
            arp->sha,
            arp->spa
        );


        return;
    }



    /*
        ARP reply
    */

    if(arp->oper == arp_ntohs(ARP_REPLY))
    {
        kprintf("[ARP] Reply\n");


        /*
            Save MAC for ping
        */

        if(arp->spa == ping_ip)
        {
            memcpy(
                ping_mac,
                arp->sha,
                6
            );


            ping_arp_done = 1;


            kprintf(
                "[ARP] MAC resolved\n"
            );


            kprintf(
                "MAC: "
            );

            arp_print_mac(
                ping_mac
            );

            kprintf("\n");
        }


        return;
    }



    kprintf(
        "[ARP] Unknown operation\n"
    );
}

void e1000_dump() {
    e1000_init();
    e1000_dump_pci();
    e1000_dump_registers();
    e1000_dump_mac();
    e1000_dump_link();
    // e1000_ping_host(
    //     0x0202000A
    // ); Gets timeout ARP
    e1000_dump_rx();
    e1000_dump_tx();
}
