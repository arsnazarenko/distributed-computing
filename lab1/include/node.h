#ifndef LAB1_NODE_H
#define LAB1_NODE_H

#include "inttypes.h"
#include "context.h"

typedef struct {
    int8_t id;
    adjacent_list neighbours;
} node;

void node_create(node *node, int8_t id, context *ctx);

void node_destroy(node *node);

#endif //LAB1_NODE_H
