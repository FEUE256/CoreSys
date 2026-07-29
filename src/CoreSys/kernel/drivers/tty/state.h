#pragma once

#define TTY_MAX_TERMS 2 // Max 2 terminals 
#define TTY_BUF_SIZE  32767

typedef struct {
    char    buffer[TTY_BUF_SIZE];
    char    cwd[256];
    int     index;
    int     active;
    int     cancel;
} tty_state_t;

static tty_state_t tty_terms[TTY_MAX_TERMS];
static int         tty_current = 0;

#define TREE_MAX_VISITED 8092

static uint64_t tree_visited[TREE_MAX_VISITED];
static int tree_visited_count = 0;
