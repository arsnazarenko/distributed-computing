#ifndef LAB1_CONTEXT_H
#define LAB1_CONTEXT_H
#include <unistd.h>
#include "pipes_util.h"

typedef struct context {
    size_t sz;
    duplex_pipe pipe_table[N_PROC][N_PROC];
} context;

int context_create(context *ctx, size_t proc_n);

void context_destroy(context *ctx);

void context_create_adjacent_list(context *ctx, size_t row_number, adjacent_list *adj_list);

#endif //LAB1_CONTEXT_H
