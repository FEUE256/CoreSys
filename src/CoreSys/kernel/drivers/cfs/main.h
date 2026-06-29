#pragma once

#include <drivers/page/main.h>
#include <stdint.h>
#include <drivers/task/main.h>
#include <globe.h>
#include <kernel/mem.h>
#include <drivers/serial/main.h>

// CoreSys Filesystem (cfs)

// cfs_MAX_NAME, cfs_MAX_CHILDREN, cfs_type and cfs_node is all defined in mod/globe.h

typedef enum {
    CFS_KRN_CFG_DEF = 0,
    CFS_KRN_CFG_DBG,
    CFS_KRN_CFG_SLT
} cfs_krn_cfg_e;

void cfs_krn_cfg(cfs_krn_cfg_e cfg, char buf[]) {
    switch (cfg)
        {
        case CFS_KRN_CFG_DEF:
            strcpy(buf, "cs_default");
            break;
        case CFS_KRN_CFG_DBG:
            strcpy(buf, "cs_debug");
            break;
        case CFS_KRN_CFG_SLT:
            strcpy(buf, "cs_silent");
            break;
        default:
            strcpy(buf, "cs_unknown");
            break;
        }
}

// ---------------- CREATE ----------------

cfs_node* cfs_create_node(const char* name, cfs_type type) {
    cfs_node* node = (cfs_node*)kmalloc(sizeof(cfs_node));

    for (int i = 0; i < cfs_MAX_NAME - 1 && name[i]; i++)
        node->name[i] = name[i];

    node->type = type;
    node->size = 0;
    node->data = 0;
    node->parent = 0;
    node->child_count = 0;

    return node;
}

static void cfs_destroy_node(cfs_node* node) {
    if (!node) return;

    // Free children first
    if (node->type == cfs_DIR) {
        for (int i = 0; i < cfs_MAX_CHILDREN; i++) {
            if (node->children[i]) {
                cfs_destroy_node(node->children[i]);
                node->children[i] = NULL;
            }
        }
    }

    // Free file data
    if (node->type == cfs_FILE && node->data) {
        kfree(sizeof(node));
        node->data = NULL;
    }

    kfree(sizeof(node));
}

// ---------------- DIRECTORY ADD ----------------

int cfs_add_child(cfs_node* parent, cfs_node* child) {
    if (!parent || parent->type != cfs_DIR) return -1;
    if (parent->child_count >= cfs_MAX_CHILDREN) return -2;

    parent->children[parent->child_count++] = child;
    child->parent = parent;
    return 0;
}

// ---------------- PATH SEARCH ----------------

cfs_node* cfs_find(cfs_node* start, const char* name) {
    if (!start) return 0;

    for (uint32_t i = 0; i < start->child_count; i++) {
        cfs_node* c = start->children[i];

        int match = 1;
        for (int j = 0; name[j] || c->name[j]; j++) {
            if (name[j] != c->name[j]) {
                match = 0;
                break;
            }
        }

        if (match) return c;
    }

    return 0;
}

// ---------------- CREATE DIR ----------------

cfs_node* cfs_mkdir(cfs_node* parent, const char* name) {
    cfs_node* dir = cfs_create_node(name, cfs_DIR);
    cfs_add_child(parent, dir);
    return dir;
}

// ---------------- CREATE FILE ----------------

cfs_node* cfs_create_file(cfs_node* parent, const char* name) {
    cfs_node* file = cfs_create_node(name, cfs_FILE);
    cfs_add_child(parent, file);
    return file;
}

// ---------------- WRITE FILE ----------------

void cfs_write(cfs_node* file, uint8_t* data, uint64_t size) {
    if (!file || file->type != cfs_FILE) return;

    file->data = data;
    file->size = size;
}

// ---------------- READ FILE ----------------

uint8_t* cfs_read(cfs_node* file, uint64_t* size_out) {
    if (!file || file->type != cfs_FILE) return 0;

    *size_out = file->size;
    return file->data;
}
   
    static cfs_node* kernel_cfg_file __attribute__((unused));

// ---------------- INIT / DEINIT FILESYSTEM ----------------

void cfs_init(cs_task* self) {
    volatile unum8_t *di = (volatile unum8_t*)KDI;
    num_t dn = (num_t)(*di);

    (void)self; // Unused parameter
    cfs_root = cfs_create_node("/", cfs_DIR);

    sys_dir = cfs_mkdir(cfs_root, "sys");
    kernel_dir = cfs_mkdir(sys_dir, "kernel");
    system_dir = cfs_mkdir(sys_dir, "system");
    (void)system_dir; // Unused variable to prevent warnings
    kernel_cfg_file = cfs_create_file(kernel_dir, "kernel.cfg");
    debug_cfg_file = cfs_create_file(system_dir, "debug.cfg");
    static char kernel_cfg_data[4092];
    static char debug_cfg_data[256];

    char buf[64] = {0};
    cfs_krn_cfg_e cfg = (cfs_krn_cfg_e)dn;
    cfs_krn_cfg(cfg, buf);

    // Buffer max: 4092
    snprintf(kernel_cfg_data, sizeof(kernel_cfg_data),
        "kernel_config=%s debug=%d",
        buf, dn);

    cfs_write(kernel_cfg_file,
            (uint8_t*)kernel_cfg_data,
            kstrlen(kernel_cfg_data));

    // Buffer max: 256 
    snprintf(debug_cfg_data, sizeof(debug_cfg_data),
        "%d",
        dn);

    cfs_write(debug_cfg_file,
            (uint8_t*)debug_cfg_data,
            kstrlen(debug_cfg_data));
}

/*
File stucture:
/
├── sys
    ├── kernel
         ├── kernel.cfg
    ├── system
         ├── debug.cfg
*/

void cfs_deinit(cs_task* self) {
    (void)self; // Unused parameter
    if (!cfs_root) return;

    cfs_destroy_node(cfs_root);
    cfs_root = NULL;
}
