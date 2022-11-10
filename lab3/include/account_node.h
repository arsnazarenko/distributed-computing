#ifndef LAB2_ACCOUNT_NODE_H
#define LAB2_ACCOUNT_NODE_H
#include "node.h"

typedef struct account_node {
    node child_node;
    BalanceState current_state;
    BalanceHistory history;
} account_node;


void account_create(account_node *account, balance_t start_balance, local_id id, context *ctx);

void account_first_phase(account_node *account);
void account_second_phase(account_node *account);
void account_third_phase(account_node *account);

void account_destroy(account_node *account);



#endif //LAB2_ACCOUNT_NODE_H
