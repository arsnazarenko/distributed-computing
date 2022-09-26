#ifndef LAB1_CONTEXT_H
#define LAB1_CONTEXT_H
#include <inttypes.h>
#define N_PROC 10

typedef union {
    int fds[2];
    struct {
        int fd_read;
        int fd_write;
    };
} half_duplex_pipe;

typedef struct {
    half_duplex_pipe input_pipe;
    half_duplex_pipe output_pipe;
} duplex_pipe;

typedef struct context {
    size_t sz;
    duplex_pipe pipe_table[N_PROC][N_PROC];
} context;

typedef struct {
    int fd_read;
    int fd_write;
} node_interface;

typedef struct {
    size_t sz;
    node_interface interfaces[N_PROC];
} adjacent_list;

int context_create(context *ctx, size_t proc_n);

void context_destroy(context *ctx);

void context_create_adjacent_list(context *ctx, size_t row_number, adjacent_list *adj_list);

#endif //LAB1_CONTEXT_H
