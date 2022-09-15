#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "node.h"

//static void node_debug(const node *node) {
//    printf("{\n"
//           "id: %d,\n"
//           "{ sz: %zu { ", node->id, node->neighbours.sz);
//    for (size_t i = 0; i < node->neighbours.sz; ++i) {
//        printf("(%d, %d) ",
//               node->neighbours.interfaces[i].fd_read,
//               node->neighbours.interfaces[i].fd_write);
//    }
//    printf("}\n}\n");
//}

void node_create(node *node, uint8_t id, context *ctx) {
    assert(node != NULL && ctx != NULL);
    assert(id >= 0 && id < 11);
    size_t row = (size_t) id;
    node->id = id;
    node->neighbours.sz = ctx->sz;
    for (size_t i = 0; i < ctx->sz; ++i) {
        // copy needed fd's to communicate for each processes in system
        node->neighbours.interfaces[i] = (node_interface)
                {.fd_read = ctx->pipe_table[row][i].input_pipe.fd_out,
                        .fd_write =  ctx->pipe_table[row][i].output_pipe.fd_in};
        // clean copied fd's from global table to prevent them from closing in context_destroy()
        ctx->pipe_table[row][i].input_pipe.fd_out = -1;
        ctx->pipe_table[row][i].output_pipe.fd_in = -1;
        ctx->pipe_table[i][row].output_pipe.fd_out = -1;
        ctx->pipe_table[i][row].input_pipe.fd_in = -1;
    }
}

void node_destroy(node *node) {
    assert(node != NULL);
    for (size_t i = 0; i < node->neighbours.sz; ++i) {
        if (i == (size_t) node->id) { continue; }
        close(node->neighbours.interfaces[i].fd_write);
        close(node->neighbours.interfaces[i].fd_read);
    }
}
