#include <errno.h>
#include <string.h>
#include "client_node.h"
#include "logger.h"
#include "lamport_time.h"

void client_create(client_node *client, local_id id, context *ctx) {
    node_create(&(client->parent_node), id, ctx);
}

void client_destroy(client_node *client) {
    node_destroy(&(client->parent_node));
}

void client_wait_all_msg(client_node *client, MessageType type, msg_handler on_receive, all_msg_handler on_receive_all) {
    const size_t account_node_number = client->parent_node.neighbours.sz - 1;
    Message msg;
    size_t received = 0;
    while(received != account_node_number) {
        int res = receive_any(&(client->parent_node), &msg);
        if (res == -1 && errno == EAGAIN) { continue; }
        if (type == (MessageType) msg.s_header.s_type) {
            sync_lamport_time(msg.s_header.s_local_time);
            inc_lamport_time();
            ++received;
            if (on_receive != NULL) {
                on_receive(client, &msg);
            }
        }
    }
    if (on_receive_all != NULL) {
        on_receive_all(client);
    }
}

void client_handle_all_start(client_node *client) {
    log_received_all_started(client->parent_node.id);
}

void client_handle_all_done(client_node *client) {
    log_received_all_done(client->parent_node.id);
}

void client_first_phase(client_node *client) {
    client_wait_all_msg(client, STARTED, NULL, client_handle_all_start);
}
void client_third_phase(client_node *client) {
    client_wait_all_msg(client, DONE, NULL, client_handle_all_done);
}
