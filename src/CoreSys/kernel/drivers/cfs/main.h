#pragma once

#include <drivers/page/main.h>
#include <stdint.h>

// CoreSys Filesystem (cfs) – minimal kernel design skeleton

#include <stdint.h>

#define cfs_MAX_NAME 64
#define cfs_MAX_CHILDREN 16

typedef enum {
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

// ---------------- ROOT ----------------

static cfs_node* cfs_root = 0;

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
        kfree(node->data);
        node->data = NULL;
    }

    kfree(node);
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

void cfs_init() {
    cfs_root = cfs_create_node("/", cfs_DIR);

    cfs_node* sys_dir = cfs_mkdir(cfs_root, "sys");
    cfs_node* kernel_dir = cfs_mkdir(sys_dir, "kernel");
    cfs_node* system_dir = cfs_mkdir(sys_dir, "system");
    (void)system_dir; // Unused variable to prevent warnings
    cfs_node* kernel_cfg_file = cfs_create_file(kernel_dir, "kernel.cfg");

    static uint8_t kernel_cfg_data[] = "kernel_config=coresys_default";
    cfs_write(kernel_cfg_file, kernel_cfg_data, sizeof(kernel_cfg_data));
}

void cfs_deinit() {
    if (!cfs_root) return;

    cfs_destroy_node(cfs_root);
    cfs_root = NULL;
}
