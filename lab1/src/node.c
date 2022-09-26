#include <assert.h>
#include <unistd.h>
#include "node.h"

void node_create(node *node, int8_t id, context *ctx) {
    assert(node != NULL && ctx != NULL);
    assert(id >= 0 && id < N_PROC);
    node->id = id;
    context_create_adjacent_list(ctx, id, &(node->neighbours));
}

void node_destroy(node *node) {
    assert(node != NULL);
    for (size_t i = 0; i < node->neighbours.sz; ++i) {
        if (i == (size_t) node->id) { continue; }
        (node->neighbours.interfaces[i].fd_write);
        close(node->neighbours.interfaces[i].fd_read);
    }
}
