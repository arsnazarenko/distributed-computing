#include "client_node.h"
#include "logger.h"
#include "lamport_time.h"

static void client_loop_break(client_node *client) {
    client->state_flags.break_flag = true;
}

void client_create(client_node *client, local_id id, context *ctx, const client_recv_handler *handlers) {
    node_create(&(client->parent_node), id, ctx);
    client->handlers = handlers;
    client->state_flags = (struct client_state_flags) {
        .done_received = 0,
        .started_received = 0,
    };
}

void client_destroy(client_node *client) {
    node_destroy(&(client->parent_node));
}

void client_loop_start(client_node *client) {
    client->state_flags.break_flag = false;
    Message msg;
    local_id from_id = 0;
    while(!client->state_flags.break_flag) {
        from_id %= client->parent_node.neighbours.sz;
        if (from_id != PARENT_ID) {
            if (receive(&(client->parent_node), from_id, &msg) != -1) {
                sync_lamport_time(msg.s_header.s_local_time);
                inc_lamport_time();
                client_recv_handler on_receive = client->handlers[msg.s_header.s_type];
                if (on_receive) {
                    on_receive(client, &msg);
                }
            }
        }
        ++from_id;
    }
}


void client_handle_done(client_node *client, Message *message) {
    (void) message;
    ++(client->state_flags.done_received);
    if (client->state_flags.done_received == (client->parent_node.neighbours.sz - 1)) {
        log_received_all_done(PARENT_ID);
        client_loop_break(client);
    }
}

void client_handle_started(client_node *client, Message *message) {
    (void) message;
    ++(client->state_flags.started_received);
    if (client->state_flags.started_received == (client->parent_node.neighbours.sz - 1)) {
        log_received_all_started(PARENT_ID);
        client_loop_break(client);
    }
}

void client_start_phase(client_node *client) {
    client_loop_start(client);
}

void client_done_phase(client_node *client) {
    client_loop_start(client);
}
