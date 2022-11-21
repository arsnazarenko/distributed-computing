#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "account_node.h"
#include "logger.h"
#include "pa2345.h"
#include "lamport_time.h"


void account_send_start_to_all(account_node *account);

void account_send_done_to_all(account_node *account);

void account_send_ack(account_node *account);

void account_send_history(account_node *account);

void account_forward_transfer(account_node *account, Message *message);

void account_balance_state_init(account_node *account, balance_t start_balance);

void account_update_balance(account_node *account, timestamp_t timestamp, balance_t amount);

void account_history_init(account_node *account, BalanceState init_state);

void account_update_history_pending(account_node *account, timestamp_t sender_timestamp, balance_t amount);


void account_create(account_node *account, balance_t start_balance, local_id id, context *ctx) {
    account->loop_stop_flag = false;
    account->event_state.done_received = 0;
    account->event_state.started_received = 0;
    account->event_state.stop_received = false;
    node_create(&(account->node), id, ctx);
    account_balance_state_init(account, start_balance);
    account_history_init(account, account->current_state);
}

void account_destroy(account_node *account) {
    node_destroy(&(account->node));
}

void account_run(account_node *account, const receive_handler *handlers) {
    account_send_start_to_all(account);
    Message msg;
    local_id from_id = 0;
    while (!account->loop_stop_flag) {
        from_id %= account->node.neighbours.sz;
        if (from_id != account->node.id) {
            if (receive(&(account->node), from_id, &msg) != -1) {
                sync_lamport_time(msg.s_header.s_local_time);
                inc_lamport_time();
                receive_handler on_receive = handlers[msg.s_header.s_type];
                if (on_receive) {
                    on_receive(account, &msg);
                }
            }
        }
        ++from_id;
    }
}

void account_stop(account_node *account) {
    account->loop_stop_flag = true;
}

void account_balance_state_init(account_node *account, balance_t start_balance) {
    account->current_state.s_time = get_lamport_time();
    account->current_state.s_balance = start_balance;
    account->current_state.s_balance_pending_in = 0;
}

void account_history_init(account_node *account, BalanceState init_state) {
    account->history.s_id = account->node.id;
    memset(&(account->history.s_history[0]), 0, sizeof(account->history.s_history));
    account->history.s_history[0] = init_state;
    account->history.s_history_len = 1;
}

void account_history_update(account_node *account, BalanceState new_state) {
    BalanceHistory *history = &(account->history);
    assert(history->s_history_len > 0);
    const timestamp_t last_time = history->s_history[history->s_history_len - 1].s_time;
    const timestamp_t new_time = new_state.s_time;
    assert(new_time >= last_time);
    for (int t = last_time + 1; t < new_time; ++t) {
        history->s_history[t] = history->s_history[t - 1];
        history->s_history[t].s_time = t;
    }
    history->s_history[new_time] = new_state;
    history->s_history_len += (new_time - last_time);
}

void account_update_balance(account_node *account, timestamp_t time, balance_t amount) {
    account->current_state.s_time = time;
    account->current_state.s_balance += amount;
    account->current_state.s_balance_pending_in = 0;
    account_history_update(account, account->current_state);
}

void account_update_history_pending(account_node *account, timestamp_t sender_timestamp, balance_t amount) {
    assert(sender_timestamp < account->current_state.s_time);
    // sender_timestamp - 1 because:
    // in timestamp "t" sender receive TRANSFER msg from client and decrease his balance
    // in timestamp "t + 1" sender send TRANSFER msg to receiver (s_local_time in header = "t + 1")
    // in timestamp "t + 2" receiver receive TRANSFER msg (with s_local_time in header = "t + 1") and increase his balance
    // in timestamp "t" sender already haven't part of balance, we should include this part in receiver account_state (balance_pending_in);
    timestamp_t pending_start = sender_timestamp - 1;
    for (timestamp_t i = pending_start; i < account->current_state.s_time; ++i) {
        account->history.s_history[i].s_balance_pending_in = amount;
    }
}

void account_handle_transfer(account_node *account, Message *message) {
    TransferOrder *transfer_order = (TransferOrder *) &(message->s_payload[0]);
    if (transfer_order->s_src == account->node.id) {
        account_update_balance(account, get_lamport_time(), -transfer_order->s_amount);
        log_transfer_out(account->node.id, transfer_order->s_amount, transfer_order->s_dst);
        account_forward_transfer(account, message);
    }
    if (transfer_order->s_dst == account->node.id) {
        account_update_balance(account, get_lamport_time(), transfer_order->s_amount);
        account_update_history_pending(account, message->s_header.s_local_time, transfer_order->s_amount);
        log_transfer_in(transfer_order->s_src, transfer_order->s_amount, account->node.id);
        account_send_ack(account);
    }
}

void account_handle_stop(account_node *account, Message *message) {
    (void) message;
    account_send_done_to_all(account);
    account->event_state.stop_received = true;
    if (account->event_state.done_received == account->node.neighbours.sz - 2) {
        account_stop(account);
        account_update_balance(account, get_lamport_time(), 0);
        account_send_history(account);
    }
}


void account_handle_started(account_node *account, Message *message) {
    (void) message;
    ++(account->event_state.started_received);
    if (account->event_state.started_received == account->node.neighbours.sz - 2) {
        log_received_all_started(account->node.id);
    }
}

void account_handle_done(account_node *account, Message *message) {
    (void) message;
    ++(account->event_state.done_received);
    if (account->event_state.done_received == account->node.neighbours.sz - 2) {
        log_received_all_done(account->node.id);
        if (account->event_state.stop_received) {
            account_stop(account);
            account_update_balance(account, get_lamport_time(), 0);
            account_send_history(account);
        }
    }
}


void account_forward_transfer(account_node *account, Message *message) {
    timestamp_t timestamp = inc_lamport_time();
    TransferOrder *transfer_order = (TransferOrder *) &(message->s_payload[0]);
    message->s_header.s_local_time = timestamp;
    while (send(&(account->node), transfer_order->s_dst, message) != 0);
}

void account_send_start_to_all(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    log_started(account->node.id, getpid(), getppid(), account->current_state.s_balance);
    Message msg;
    sprintf(msg.s_payload, log_started_fmt, timestamp, account->node.id, getpid(), getppid(),
            account->current_state.s_balance);
    msg.s_header = (MessageHeader) {
            .s_type = STARTED,
            .s_payload_len = strlen(msg.s_payload),
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->node), &msg);
}

void account_send_done_to_all(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    log_done(account->node.id, account->current_state.s_balance);
    Message msg;
    sprintf(msg.s_payload, log_done_fmt, timestamp, account->node.id, account->current_state.s_balance);
    msg.s_header = (MessageHeader) {
            .s_type = DONE,
            .s_payload_len = strlen(msg.s_payload),
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->node), &msg);
}

void account_send_ack(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = ACK,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    while (send(&(account->node), PARENT_ID, &msg) != 0);
}

void account_send_history(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    Message msg;
    size_t history_buf_used_size = account->history.s_history_len * sizeof(BalanceState);
    size_t payload_len = sizeof(account->history) - (sizeof(account->history.s_history) - history_buf_used_size);
    memcpy(msg.s_payload, &(account->history), payload_len);
    msg.s_header = (MessageHeader) {
            .s_type = BALANCE_HISTORY,
            .s_payload_len = payload_len,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    while (send(&(account->node), PARENT_ID, &msg) != 0);
}

