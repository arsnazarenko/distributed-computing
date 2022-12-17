#ifndef LAB1_CONTEXT_H
#define LAB1_CONTEXT_H

#include <unistd.h>
#include "arg_utils.h"
#include "ipc.h"
#include "pipe_utils.h"

typedef struct node_interface {
    int fd_read;
    int fd_write;
} node_interface;

typedef struct adjacent_list {
    size_t sz;
    node_interface interfaces[MAX_PROC_ID + 1];
} adjacent_list;

typedef struct context {
    size_t sz;
    duplex_pipe pipe_table[MAX_PROC_ID + 1][MAX_PROC_ID + 1];
} context;

int context_create(context *ctx, size_t proc_n);

void context_destroy(context *ctx);

void context_create_adjacent_list(context *ctx, local_id id, adjacent_list *adj_list);

#endif //LAB1_CONTEXT_H
