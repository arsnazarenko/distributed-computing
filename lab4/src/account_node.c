#include <stdbool.h>
#include <string.h>
#include "account_node.h"
#include "logger.h"
#include "pa2345.h"
#include "lamport_time.h"


void account_create(account_node *account, local_id id, context *ctx, const receive_handler *handlers) {
    account->handlers = handlers;
    account->event_state.done_received = 0;
    account->event_state.started_received = 0;
    account->req_queue = vector_init();
    node_create(&(account->node), id, ctx);
}

void account_destroy(account_node *account) {
    vector_destroy(account->req_queue);
    node_destroy(&(account->node));
}

static void account_loop_break(account_node *account) {
    account->event_state.break_flag = true;
}

void account_loop_start(account_node *account) {
    account->event_state.break_flag = false;
    Message msg;
    local_id from_id = 0;
    while (!account->event_state.break_flag) {
        from_id %= account->node.neighbours.sz;
        if (from_id != account->node.id) {
            if (receive(&(account->node), from_id, &msg) != -1) {
                sync_lamport_time(msg.s_header.s_local_time);
                inc_lamport_time();
                receive_handler on_receive = account->handlers[msg.s_header.s_type];
                if (on_receive) {
                    on_receive(account, &msg);
                }
            }
        }
        ++from_id;
    }
}

void account_handle_started(account_node *account, Message *message) {
    (void) message;
    ++(account->event_state.started_received);
    if (account->event_state.started_received == account->node.neighbours.sz - 2) {
        log_received_all_started(account->node.id);
        account_loop_break(account);
    }
}

void account_handle_done(account_node *account, Message *message) {
    (void) message;
    ++(account->event_state.done_received);
    if (account->event_state.done_received == account->node.neighbours.sz - 2) {
        log_received_all_done(account->node.id);
        account_loop_break(account);
    }
}

static bool is_cs_candidate(account_node *account) {
    const key* front = vector_front(account->req_queue);
    bool in_queue_head = front->id == account->node.id;
    bool all_replies_received = (account->event_state.reply_received == account->node.neighbours.sz - 2);
    return (in_queue_head && all_replies_received);
}

void account_handle_reply(account_node *account, Message *message) {
    /**
     * todo:
     * sync lamport time
     * inc reply msg count
     * check if this process is cs candidate:
     * if (is_cs_candidate) { account_loop_break(account); return; } and enter in CS
     */
}

void account_handle_release(account_node *account, Message *message) {
    /**
     * todo:
     * sync lamport time
     * pop from req_queue front
     * check if this process is cs candidate:
     * if (is_cs_candidate) { account_loop_break(account); return; } and enter in CS
     */
}

void account_handle_request(account_node *account, Message *message) {
    /**
     * todo:
     * sync lamport time
     * push in req_queue front
     * send reply
     */
}



static void account_send_start_to_all(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    log_started(account->node.id, getpid(), getppid());
    Message msg;
    sprintf(msg.s_payload, log_started_fmt, timestamp, account->node.id, getpid(), getppid(), 0);
    msg.s_header = (MessageHeader) {
            .s_type = STARTED,
            .s_payload_len = strlen(msg.s_payload),
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->node), &msg);
}

static void account_send_done_to_all(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    log_done(account->node.id);
    Message msg;
    sprintf(msg.s_payload, log_done_fmt, timestamp, account->node.id, 0);
    msg.s_header = (MessageHeader) {
            .s_type = DONE,
            .s_payload_len = strlen(msg.s_payload),
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->node), &msg);
}

static void account_put_in_queue(account_node *account, key k) {
    vector_push_back(account->req_queue, k);
    vector_sort(account->req_queue, key_compare);
}

void account_request_cs(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    key k = {.id = account->node.id, .time = timestamp};
    account_put_in_queue(account, k);
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = CS_REQUEST,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC };
    send_multicast(&(account->node), &msg);
    account_loop_start(account); // block on receiving messages until we can enter to cs
}

void account_release_cs(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = CS_RELEASE,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->node), &msg);
}

void account_reply(account_node *account, local_id dst) {
    timestamp_t timestamp = inc_lamport_time();
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = CS_REPLY,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC };
    send(&(account->node), dst, &msg);
}



void account_first_phase(account_node *account) {
    account_send_start_to_all(account);
    account_loop_start(account);
}
void account_third_phase(account_node *account) {
    account_send_done_to_all(account);
    account_loop_start(account);
}
