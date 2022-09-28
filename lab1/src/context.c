#include <assert.h>
#include "context.h"
#include "logger.h"

int context_create(context *ctx, size_t proc_n) {
    assert(proc_n <= N_PROC);
    assert(ctx != NULL);
    ctx->sz = proc_n;
    for (size_t i = 0; i < ctx->sz; ++i) {
        for (size_t j = i + 1; j < ctx->sz; ++j) {
            half_duplex_pipe input_p = {0};
            half_duplex_pipe output_p = {0};
            if (pipe(input_p.fds) != 0 || pipe(output_p.fds) != 0) {
                perror("pipe() failed");
                // if some number of pipes was opened
                context_destroy(ctx);
                return -1;
            }
            log_pipe_open(input_p);
            log_pipe_open(output_p);
            ctx->pipe_table[i][j] = (duplex_pipe) {input_p, output_p};
            ctx->pipe_table[j][i] = (duplex_pipe) {output_p, input_p};
        }
    }
    return 0;
}

void context_destroy(context *ctx) {
    assert(ctx != NULL);
    for (size_t i = 0; i < ctx->sz; ++i) {
        for (size_t j = i + 1; j < ctx->sz; ++j) {
            close_pipe(ctx->pipe_table[i][j].output_pipe);
            close_pipe(ctx->pipe_table[i][j].input_pipe);
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
                 .fd_write =  ctx->pipe_table[row_number][i].output_pipe.fd_write };
        // clean copied fd's from global table to prevent them from closing in context_destroy()
        ctx->pipe_table[row_number][i].input_pipe.fd_read = 0;
        ctx->pipe_table[row_number][i].output_pipe.fd_write = 0;
        ctx->pipe_table[i][row_number].output_pipe.fd_read = 0;
        ctx->pipe_table[i][row_number].input_pipe.fd_write = 0;
    }
}





