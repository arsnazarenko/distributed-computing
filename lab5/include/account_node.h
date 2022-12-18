#ifndef LAB2_ACCOUNT_NODE_H
#define LAB2_ACCOUNT_NODE_H

#include "node.h"
#include "lamport_time.h"

typedef struct account_node account_node;
typedef void (*receive_handler)(account_node *account, Message *message, local_id from);

struct account_node {
    node node;
    lamport_key current_request_time;
    const receive_handler *handlers;
    struct state_flags {
        size_t started_received;
        size_t done_received;
        size_t reply_received;
        bool break_flag;
        bool work_done_flag;
    } state_flags;
    struct delayed_replies {
        size_t sz;
        uint8_t arr[MAX_PROC_ID + 1];
    } delayed_replies;
};

void account_create(account_node *account, local_id id, context *ctx, const receive_handler *handlers);
void account_destroy(account_node *account);

void account_handle_started(account_node *account, Message *message, local_id from);
void account_handle_done(account_node *account, Message *message, local_id from);
void account_handle_reply(account_node *account, Message *message, local_id from);
void account_handle_request(account_node *account, Message *message, local_id from);

void account_request_cs(account_node *account);
void account_release_cs(account_node *account);

void account_start_phase(account_node *account);
void account_done_phase(account_node *account);

#endif //LAB2_ACCOUNT_NODE_H
