#ifndef LAB2_CLIENT_NODE_H
#define LAB2_CLIENT_NODE_H
#include "node.h"
#include "banking.h"

typedef struct client_node {
    node parent_node;
    AllHistory all_history;
} client_node;


void client_create(client_node *client, local_id id, context *ctx);
void client_destroy(client_node *client);

void client_transfer(client_node *client, local_id src, local_id dst, balance_t amount);
void client_first_phase(client_node *client);
void client_third_phase(client_node *client);

#endif //LAB2_CLIENT_NODE_H
