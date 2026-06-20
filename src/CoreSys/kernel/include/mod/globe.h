#pragma once 

#include <types.h>
#include <status.h>

// For cfs_node*
#define cfs_MAX_NAME 64
#define cfs_MAX_CHILDREN 16

typedef enum cfs_type { 
    cfs_FILE, 
    cfs_DIR 
} cfs_type;

typedef struct cfs_node {
    char name[cfs_MAX_NAME];
    cfs_type type;

    uint64_t size;
    uint8_t* data;

    struct cfs_node* parent;

    struct cfs_node* children[cfs_MAX_CHILDREN];
    uint32_t child_count;
} cfs_node;

num_t tsk_ready = 0;

CS_STATUS_T status; // Status
CS_SUBSYS_STATUS_T sstatus;

static cfs_node* cfs_root = 0;

static cfs_node* sys_dir = 0;
static cfs_node* kernel_dir = 0;
static cfs_node* system_dir = 0;
static cfs_node* kernel_cfg_file = 0;
static cfs_node* debug_cfg_file = 0;

uint32_t bar2;
uintptr_t fb;
volatile uint32_t* framebuffer;