#ifndef LAB2_ACCOUNT_NODE_H
#define LAB2_ACCOUNT_NODE_H

#include "node.h"
#include "vector.h"


typedef struct account_node account_node;

typedef void (*receive_handler)(account_node *account, Message *message, local_id from);

struct account_node {
    node node;
    struct {
        size_t started_received;
        size_t done_received;
        size_t reply_received;
        bool break_flag;
        bool work_done_flag;
    } event_state;
    const receive_handler *handlers;
    vector_t req_queue;
};


void account_create(account_node *account, local_id id, context *ctx, const receive_handler *handlers);
void account_destroy(account_node *account);

void account_handle_started(account_node *account, Message *message, local_id from);
void account_handle_done(account_node *account, Message *message, local_id from);
void account_handle_reply(account_node *account, Message *message, local_id from);
void account_handle_release(account_node *account, Message *message, local_id from);
void account_handle_request(account_node *account, Message *message, local_id from);

void account_request_cs(account_node *account);
void account_release_cs(account_node *account);

void account_start_phase(account_node *account);
void account_done_phase(account_node *account);

#endif //LAB2_ACCOUNT_NODE_H
