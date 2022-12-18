#ifndef LAB2_CLIENT_NODE_H
#define LAB2_CLIENT_NODE_H
#include "node.h"

typedef struct client_node client_node;
typedef void (*client_recv_handler)(client_node *account, Message *message);

struct client_node {
    node parent_node;
    const client_recv_handler *handlers;
    struct client_state_flags {
        size_t started_received;
        size_t done_received;
        bool break_flag;
    } state_flags;
};


void client_create(client_node *client, local_id id, context *ctx, const client_recv_handler *handlers);
void client_destroy(client_node *client);

void client_handle_started(client_node *client, Message *message);
void client_handle_done(client_node *client, Message *message);

void client_start_phase(client_node *client);
void client_done_phase(client_node *client);

#endif //LAB2_CLIENT_NODE_H
