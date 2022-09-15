#ifndef LAB1_CONTEXT_H
#define LAB1_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PROC_SIZE 11

typedef union {
    int fds[2];
    struct {
        int fd_in;
        int fd_out;
    };
} half_duplex_pipe;


typedef struct {
    half_duplex_pipe input_pipe;
    half_duplex_pipe output_pipe;
} duplex_pipe;


typedef struct {
    size_t sz;
    duplex_pipe pipe_table[MAX_PROC_SIZE][MAX_PROC_SIZE];
} context;

bool context_create(context *ctx, size_t sz);

void context_destroy(context *ctx);

#endif //LAB1_CONTEXT_H
