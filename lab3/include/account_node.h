#ifndef LAB2_ACCOUNT_NODE_H
#define LAB2_ACCOUNT_NODE_H

#include "node.h"

typedef struct account_node {
    node node;
    struct {
        size_t started_received;
        size_t done_received;
        bool stop_received;
    } event_state;
    bool loop_stop_flag;
    BalanceState current_state;
    BalanceHistory history;
} account_node;

typedef void (*receive_handler)(account_node *account, Message *message);

void account_create(account_node *account, balance_t start_balance, local_id id, context *ctx);
void account_destroy(account_node *account);

void account_run(account_node *account, const receive_handler *handlers);
void account_stop(account_node *account);

void account_handle_started(account_node *account, Message *message);
void account_handle_done(account_node *account, Message *message);
void account_handle_transfer(account_node *account, Message *message);
void account_handle_stop(account_node *account, Message *message);

#endif //LAB2_ACCOUNT_NODE_H
