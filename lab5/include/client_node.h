#ifndef LAB2_CLIENT_NODE_H
#define LAB2_CLIENT_NODE_H
#include "node.h"

typedef struct client_node {
    node parent_node;
} client_node;

typedef void (*all_msg_handler)(client_node *account);
typedef void (*msg_handler)(client_node *account, Message *message);

void client_create(client_node *client, local_id id, context *ctx);
void client_destroy(client_node *client);

void client_wait_all_msg(client_node *client, MessageType type, msg_handler on_receive, all_msg_handler on_receive_all);
void client_handle_all_start(client_node *client);
void client_handle_all_done(client_node *client);


void client_first_phase(client_node *client);
void client_third_phase(client_node *client);

#endif //LAB2_CLIENT_NODE_H
