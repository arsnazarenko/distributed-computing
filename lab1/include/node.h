#ifndef LAB1_NODE_H
#define LAB1_NODE_H

#include "inttypes.h"
#include "context.h"
#include "ipc.h"

typedef struct {
    local_id id;
    adjacent_list neighbours;
} node;

void node_create(node *node, local_id id, context *ctx);

void node_destroy(node *node);

#endif //LAB1_NODE_H
