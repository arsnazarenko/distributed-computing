#ifndef LAB1_NODE_H
#define LAB1_NODE_H
#include "context.h"

typedef struct {
    int fd_read;
    int fd_write;
} node_interface;

typedef struct {
    size_t sz;
    node_interface interfaces[11];
} adjacent_list;

typedef struct {
    uint8_t id;
    adjacent_list neighbours;
} node;


void node_create(node* node, uint8_t id, context* ctx);

void node_destroy(node* node);



#endif //LAB1_NODE_H
