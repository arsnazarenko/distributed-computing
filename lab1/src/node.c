#include <assert.h>
#include "node.h"


void node_create(node *node, local_id id, context *ctx) {
    assert(node != NULL && ctx != NULL);
    assert(id > 0 && id < 11);
    node->id = id;
    node->neighbours = (adjacent_list) { ctx->sz, {0} };
    size_t row = (size_t) id;
    for (size_t i = 0; i < ctx->sz; ++i) {
        ctx->pipe_table[row][i].input_pipe.fd_out;
        // fixme:
    }

}

void node_destroy(node *node) {

}