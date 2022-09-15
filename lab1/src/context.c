#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include "context.h"


static void close_hdp(half_duplex_pipe hdp) {
    if (hdp.fd_in > 3) { close(hdp.fd_in); }
    if (hdp.fd_out > 3) { close(hdp.fd_out); }
}

//void context_debug(const context* ctx) {
//    printf("{\nsz: %zu\n", ctx->sz);
//    for (size_t i = 0; i < MAX_PROC_SIZE; ++i) {
//        for (size_t j = 0; j < MAX_PROC_SIZE; ++j) {
//            printf("[%2d,%2d,%2d,%2d] ",
//                   ctx->pipe_table[i][j].input_pipe.fd_in,
//                   ctx->pipe_table[i][j].input_pipe.fd_out,
//                   ctx->pipe_table[i][j].output_pipe.fd_in,
//                   ctx->pipe_table[i][j].output_pipe.fd_out );
//        }
//        printf("\n");
//    }
//    printf("}\n");
//}

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




