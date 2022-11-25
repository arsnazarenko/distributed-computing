#ifndef LAB2_CLIENT_NODE_H
#define LAB2_CLIENT_NODE_H
#include "banking.h"
#include "node.h"

typedef struct client_node {
    node parent_node;
    AllHistory all_history;
} client_node;

typedef void (*all_msg_handler)(client_node *account);
typedef void (*msg_handler)(client_node *account, Message *message);

void client_create(client_node *client, local_id id, context *ctx);
void client_destroy(client_node *client);

void client_wait_all_msg(client_node *client, MessageType type, msg_handler on_receive, all_msg_handler on_receive_all);
void client_handle_all_start(client_node *client);
void client_handle_all_done(client_node *client);
void client_handle_history(client_node *client, Message *message);

void client_send_stop_to_all(client_node *client);

void client_first_phase(client_node *client);
void client_transfer(client_node *client, local_id src, local_id dst, balance_t amount);
void client_third_phase(client_node *client);

#endif //LAB2_CLIENT_NODE_H
