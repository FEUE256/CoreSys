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
    let_t name[cfs_MAX_NAME];
    cfs_type type;

    unum64_t size;
    unum64_t* data;

    str_t cfs_node* parent;

    str_t cfs_node* children[cfs_MAX_CHILDREN];
    unum32_t child_count;
} cfs_node;

num_t tsk_ready = 0;

CS_STATUS_T status; // status
CS_SUBSYS_STATUS_T sstatus;

sta_t cfs_node* cfs_root = 0;

sta_t cfs_node* sys_dir = 0;
sta_t cfs_node* kernel_dir = 0;
sta_t cfs_node* system_dir = 0;
sta_t cfs_node* kernel_cfg_file = 0;
sta_t cfs_node* debug_cfg_file = 0;

unum32_t bar2;
unumarr_t fb;
vol_t unum32_t* framebuffer;

#define VGA_GRAPHICS ((volatile uint8_t*)0xA0000) // Mode 13h framebuffer
#define VGA_TEXT     ((volatile uint16_t*)0xB8000) // Text mode memory
#define VGA_MONO     ((volatile uint16_t*)0xB0000) // Monochrome text
