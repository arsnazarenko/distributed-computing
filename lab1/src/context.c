#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include "context.h"

static void close_hdp(half_duplex_pipe hdp) {
    if (hdp.fd_read > 3) { close(hdp.fd_read); }
    if (hdp.fd_write > 3) { close(hdp.fd_write); }
}

bool context_create(context *ctx, size_t n_proc) {
    assert(n_proc <= MAX_PROC_SIZE);
    assert(ctx != NULL);
    ctx->sz = n_proc;
    for (size_t i = 0; i < ctx->sz; ++i) {
        for (size_t j = i + 1; j < ctx->sz; ++j) {
            half_duplex_pipe input_p = {0};
            half_duplex_pipe output_p = {0};
            if (pipe(input_p.fds) != 0 || pipe(output_p.fds) != 0) {
                perror("pipe() failed");
                return false;
            }
            ctx->pipe_table[i][j] = (duplex_pipe) {input_p, output_p};
            ctx->pipe_table[j][i] = (duplex_pipe) {output_p, input_p};
        }
    }
    return true;
}

void context_destroy(context *ctx) {
    assert(ctx != NULL);
    for (size_t i = 0; i < ctx->sz; ++i) {
        for (size_t j = i + 1; j < ctx->sz; ++j) {
            close_hdp(ctx->pipe_table[i][j].output_pipe);
            close_hdp(ctx->pipe_table[i][j].input_pipe);
        }
    }
}

void context_create_adjacent_list(context *ctx, size_t row_number, adjacent_list *adj_list) {
    assert(ctx != NULL && adj_list != NULL);
    adj_list->sz = ctx->sz;
    for (size_t i = 0; i < ctx->sz; ++i) {
        // copy needed fd's to communicate for each processes in system
        adj_list->interfaces[i] = (node_interface)
                {.fd_read = ctx->pipe_table[row_number][i].input_pipe.fd_read,
                        .fd_write =  ctx->pipe_table[row_number][i].output_pipe.fd_write};
        // clean copied fd's from global table to prevent them from closing in context_destroy()
        ctx->pipe_table[row_number][i].input_pipe.fd_read = -1;
        ctx->pipe_table[row_number][i].output_pipe.fd_write = -1;
        ctx->pipe_table[i][row_number].output_pipe.fd_read = -1;
        ctx->pipe_table[i][row_number].input_pipe.fd_write = -1;
    }
}





