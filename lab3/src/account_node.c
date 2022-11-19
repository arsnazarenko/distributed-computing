#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "account_node.h"
#include "logger.h"
#include "pa2345.h"
#include "lamport_time.h"

typedef void (*all_msg_handler)(account_node *account);
typedef void (*msg_handler)(account_node *account, Message *message);

static void account_wait_all_msg(account_node *account, MessageType type, msg_handler on_receive, all_msg_handler on_receive_all);
static void account_wait_msg(account_node *account, const msg_handler* handlers);

static void account_handle_transfer(account_node *account, Message *message);
static void account_handle_stop(account_node *account, Message *message);
static void account_handle_all_start(account_node *account);
static void account_handle_all_done(account_node *account);

static void account_send_start_to_all(account_node *account);
static void account_send_done_to_all(account_node *account);
static void account_update_balance(account_node *account, timestamp_t timestamp, balance_t amount);
static void account_send_ack(account_node *account);
static void account_send_history(account_node *account);
static void account_forward_transfer(account_node *account, Message *message);
static void account_balance_state_init(account_node *account, balance_t start_balance);
static void account_history_init(account_node *account, BalanceState init_state);
static void account_update_history_pending(account_node *account, timestamp_t sender_timestamp, balance_t amount);
static int receive_any_from_clients(void *self, Message *msg);


static const msg_handler msg_handlers[CS_RELEASE + 1] = {
        [STOP] = account_handle_stop,
        [TRANSFER] = account_handle_transfer
        // other handlers
};

static int receive_any_from_clients(void *self, Message *msg) {
    const node *self_node = (node *) self;
    const local_id max_id = (local_id) (self_node->neighbours.sz - 1);
    for (local_id id = 1; id <= max_id; ++id) {
        if (id != self_node->id) {
            if (receive(self, id, msg) != -1) { return 0; }
        }
    }
    return -1;
}


static void account_wait_all_msg(account_node *account, MessageType type, msg_handler on_receive, all_msg_handler on_receive_all) {
    const size_t account_neighbours_number = account->child_node.neighbours.sz - 2;
    Message msg;
    size_t received = 0;
    while(received != account_neighbours_number) {
        int res = receive_any_from_clients(&(account->child_node), &msg);
        if (res != 0) { continue; }
        if (type == (MessageType) msg.s_header.s_type) {
            sync_lamport_time(msg.s_header.s_local_time);
            inc_lamport_time();
            ++received;
            if (on_receive != NULL) {
                on_receive(account, &msg);
            }
        }
    }
    if (on_receive_all != NULL) {
        on_receive_all(account);
    }
}

static void account_wait_msg(account_node *account, const msg_handler* handlers) {
    Message msg;
    while(true) {
        int res = receive_any(&(account->child_node), &msg);
        if (res != 0) { continue; }
        sync_lamport_time(msg.s_header.s_local_time);
        inc_lamport_time();
        MessageType type = msg.s_header.s_type;
        handlers[type](account, &msg);
        if (type == STOP) { break; }
    }
}

static void account_balance_state_init(account_node *account, balance_t start_balance) {
    account->current_state.s_time = get_lamport_time();
    account->current_state.s_balance = start_balance;
    account->current_state.s_balance_pending_in = 0;
}

static void account_history_init(account_node *account, BalanceState init_state) {
    account->history.s_id = account->child_node.id;
    memset(&(account->history.s_history[0]), 0, sizeof(account->history.s_history));
    account->history.s_history[0] = init_state;
    account->history.s_history_len = 1;
}

void account_create(account_node *account, balance_t start_balance, local_id id, context *ctx) {
    node_create(&(account->child_node), id, ctx);
    account_balance_state_init(account, start_balance);
    account_history_init(account, account->current_state);
}

static void account_history_update(account_node *account, BalanceState new_state) {
    BalanceHistory *history = &(account->history);
    assert(history->s_history_len > 0);
    const timestamp_t last_time = history->s_history[history->s_history_len - 1].s_time;
    const timestamp_t new_time = new_state.s_time;
    assert(new_time >= last_time);
    for (int t = last_time + 1; t < new_time; ++t) {
        history->s_history[t] = history->s_history[t - 1];
        history->s_history[t].s_time =  t;
    }
    history->s_history[new_time] = new_state;
    history->s_history_len += (new_time - last_time);
}


static void account_update_balance(account_node *account, timestamp_t time, balance_t amount) {
    account->current_state.s_time = time;
    account->current_state.s_balance += amount;
    account->current_state.s_balance_pending_in = 0;
    account_history_update(account, account->current_state);
}

void account_destroy(account_node *account) {
    node_destroy(&(account->child_node));
}

static void account_forward_transfer(account_node *account, Message *message) {
    timestamp_t timestamp = inc_lamport_time();
    TransferOrder *transfer_order = (TransferOrder*) &(message->s_payload[0]);
    message->s_header.s_local_time = timestamp;
    while(send(&(account->child_node), transfer_order->s_dst, message) != 0);
}

static void account_update_history_pending(account_node *account, timestamp_t sender_timestamp, balance_t amount) {
    assert(sender_timestamp < account->current_state.s_time);
    // sender_timestamp - 1 because:
    // in timestamp "t" sender receive TRANSFER msg from client and decrease his balance
    // in timestamp "t + 1" sender send TRANSFER msg to receiver (s_local_time in header = "t + 1")
    // in timestamp "t + 2" receiver receive TRANSFER msg (with s_local_time in header = "t + 1") and increase his balance
    // in timestamp "t" sender already haven't part of balance, we should include this part in receiver state (balance_pending_in);
    timestamp_t pending_start = sender_timestamp - 1;
    for (timestamp_t i = pending_start; i < account->current_state.s_time; ++i) {
        account->history.s_history[i].s_balance_pending_in = amount;
    }
}


static void account_handle_transfer(account_node *account, Message *message) {
    TransferOrder *transfer_order = (TransferOrder*) &(message->s_payload[0]);
    if (transfer_order->s_src == account->child_node.id) {
        account_update_balance(account, get_lamport_time(),  -transfer_order->s_amount);
        log_transfer_out(account->child_node.id, transfer_order->s_amount, transfer_order->s_dst);
        account_forward_transfer(account, message);
    }
    if (transfer_order->s_dst == account->child_node.id) {
        account_update_balance(account, get_lamport_time(), transfer_order->s_amount);
        account_update_history_pending(account, message->s_header.s_local_time, transfer_order->s_amount);
        log_transfer_in(transfer_order->s_src, transfer_order->s_amount, account->child_node.id);
        account_send_ack(account);
    }
}

static void account_handle_stop(account_node *account, Message *message) {
    (void) message;
    account_update_balance(account, get_lamport_time(), 0);
}

static void account_handle_all_start(account_node *account) {
    log_received_all_started(account->child_node.id);
}

static void account_handle_all_done(account_node *account) {
    log_received_all_done(account->child_node.id);
}

static void account_send_start_to_all(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    log_started(account->child_node.id, getpid(), getppid(), account->current_state.s_balance);
    Message msg;
    sprintf(msg.s_payload, log_started_fmt, timestamp, account->child_node.id, getpid(), getppid(),
            account->current_state.s_balance);
    msg.s_header = (MessageHeader) {
            .s_type = STARTED,
            .s_payload_len = strlen(msg.s_payload),
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->child_node), &msg);
}

static void account_send_done_to_all(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    log_done(account->child_node.id, account->current_state.s_balance);
    Message msg;
    sprintf(msg.s_payload, log_done_fmt, timestamp, account->child_node.id, account->current_state.s_balance);
    msg.s_header = (MessageHeader) {
            .s_type = DONE,
            .s_payload_len = strlen(msg.s_payload),
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->child_node), &msg);
}

static void account_send_ack(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = ACK,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    while(send(&(account->child_node), PARENT_ID, &msg) != 0);
}


static void account_send_history(account_node *account) {
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
    while(send(&(account->child_node), PARENT_ID, &msg) != 0);
}

void account_first_phase(account_node *account) {
    account_send_start_to_all(account);
    account_wait_all_msg(account, STARTED, NULL, account_handle_all_start);
}

void account_second_phase(account_node *account) {
    account_wait_msg(account, msg_handlers);
}

void account_third_phase(account_node *account) {
    account_send_done_to_all(account);
    account_wait_all_msg(account, DONE, NULL, account_handle_all_done);
    account_send_history(account);
}

