#include <assert.h>
#include <fcntl.h>
#include "context.h"
#include "logger.h"

int context_create(context *ctx, size_t proc_n) {
    assert(proc_n <= (MAX_PROC_ID + 1));
    assert(ctx != NULL);
    ctx->sz = proc_n;
    for (size_t i = 0; i < ctx->sz; ++i) {
        for (size_t j = i + 1; j < ctx->sz; ++j) {
            int input_p[2];
            int output_p[2];
            if (pipe(input_p) != 0 || pipe(output_p) != 0) {
                perror("pipe() failed");
                // if some number of pipes was opened
                context_destroy(ctx);
                return -1;
            }
            fcntl(input_p[FD_READ], F_SETFL, fcntl(input_p[FD_READ], F_GETFL, 0) | O_NONBLOCK);
            fcntl(input_p[FD_WRITE], F_SETFL, fcntl(input_p[FD_WRITE], F_GETFL, 0) | O_NONBLOCK);
            fcntl(output_p[FD_READ], F_SETFL, fcntl(output_p[FD_READ], F_GETFL, 0) | O_NONBLOCK);
            fcntl(output_p[FD_WRITE], F_SETFL, fcntl(output_p[FD_WRITE], F_GETFL, 0) | O_NONBLOCK);
            log_pipe_open(input_p);
            log_pipe_open(output_p);
            ctx->pipe_table[i][j] = (duplex_pipe) {
                {input_p[FD_READ], input_p[FD_WRITE]},
                {output_p[FD_READ], output_p[FD_WRITE]}};
            ctx->pipe_table[j][i] = (duplex_pipe) {
                    {output_p[FD_READ], output_p[FD_WRITE]},
                    {input_p[FD_READ], input_p[FD_WRITE]}};
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

void context_create_adjacent_list(context *ctx, local_id id, adjacent_list *adj_list) {
    assert(ctx != NULL && adj_list != NULL);
    adj_list->sz = ctx->sz;
    for (size_t i = 0; i < ctx->sz; ++i) {
        // copy needed fd's to communicate for each processes in system
        adj_list->interfaces[i] = (node_interface)
                {.fd_read = ctx->pipe_table[id][i].input_pipe[FD_READ],
                 .fd_write =  ctx->pipe_table[id][i].output_pipe[FD_WRITE] };
        // clean copied fd's from global table to prevent them from closing in context_destroy()
        ctx->pipe_table[id][i].input_pipe[FD_READ] = 0;
        ctx->pipe_table[id][i].output_pipe[FD_WRITE] = 0;
        ctx->pipe_table[i][id].output_pipe[FD_READ] = 0;
        ctx->pipe_table[i][id].input_pipe[FD_WRITE] = 0;
    }
}





