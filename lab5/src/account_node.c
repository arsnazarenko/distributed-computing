#include <stdbool.h>
#include <string.h>
#include "account_node.h"
#include "logger.h"
#include "pa2345.h"

static const lamport_key NOT_IN_REQUEST_CS_STATE = {-1, -1};

static void account_loop_start(account_node *account);
static void account_loop_break(account_node *account);

static bool account_is_work_done(account_node *account);
static bool account_is_cs_candidate(account_node *account);

static void account_reply_cs(account_node *account, local_id dst);
static void account_send_start_to_all(account_node *account);
static void account_send_done_to_all(account_node *account);


static void account_loop_break(account_node *account) {
    account->state_flags.break_flag = true;
}


static bool account_is_work_done(account_node *account) {
    return ((account->state_flags.done_received == account->node.neighbours.sz - 2)
            &&
            account->state_flags.work_done_flag);
}

static bool account_is_cs_candidate(account_node *account) {
    return (account->state_flags.reply_received == account->node.neighbours.sz - 2);
}

static void account_loop_start(account_node *account) {
    account->state_flags.break_flag = false;
    Message msg;
    local_id from_id = 0;
    if (account->node.neighbours.sz - 2 < 1) { return;}
    while (!account_is_work_done(account) && !account->state_flags.break_flag) {
        from_id %= account->node.neighbours.sz;
        if (from_id != account->node.id) {
            if (receive(&(account->node), from_id, &msg) != -1) {
                sync_lamport_time(msg.s_header.s_local_time);
                inc_lamport_time();
                receive_handler on_receive = account->handlers[msg.s_header.s_type];
                if (on_receive) {
                    on_receive(account, &msg, from_id);
                }
            }
        }
        ++from_id;
    }
}

static void account_reply_cs(account_node *account, local_id dst) {
    timestamp_t timestamp = inc_lamport_time();
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = CS_REPLY,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    while (send(&(account->node), dst, &msg) != 0);
}

static void account_send_start_to_all(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    log_started(account->node.id, getpid(), getppid(), 0);
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
    log_done(account->node.id, 0);
    Message msg;
    sprintf(msg.s_payload, log_done_fmt, timestamp, account->node.id, 0);
    msg.s_header = (MessageHeader) {
            .s_type = DONE,
            .s_payload_len = strlen(msg.s_payload),
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->node), &msg);
}

void account_create(account_node *account, local_id id, context *ctx, const receive_handler *handlers) {
    node_create(&(account->node), id, ctx);
    account->handlers = handlers;
    account->current_request_time = NOT_IN_REQUEST_CS_STATE;
    account->state_flags = (struct state_flags) {
        .done_received = 0,
        .started_received = 0,
        .reply_received = 0,
        .work_done_flag = false,
    };
    account->delayed_replies.sz = account->node.neighbours.sz;
    memset(account->delayed_replies.arr, 0, sizeof(account->delayed_replies.arr));
}

void account_destroy(account_node *account) {
    node_destroy(&(account->node));
}

void account_handle_started(account_node *account, Message *message, local_id from) {
    (void) message;
    (void) from;
    ++(account->state_flags.started_received);
    if (account->state_flags.started_received == account->node.neighbours.sz - 2) {
        log_received_all_started(account->node.id);
        account_loop_break(account);
    }
}

void account_handle_done(account_node *account, Message *message, local_id from) {
    (void) message;
    (void) from;
    ++(account->state_flags.done_received);
    if (account->state_flags.done_received == account->node.neighbours.sz - 2) {
        log_received_all_done(account->node.id);
    }
}


void account_handle_reply(account_node *account, Message *message, local_id from) {
    (void) message;
    (void) from;
    ++(account->state_flags.reply_received);
    if (account_is_cs_candidate(account)) { account_loop_break(account); }
}

void account_handle_request(account_node *account, Message *message, local_id from) {
    const lamport_key current_req_time = account->current_request_time;
    const lamport_key received_req_time = {.id = from, .time = message->s_header.s_local_time};

    int comp_with_not_in_req_state = lamport_key_compare(&current_req_time, &NOT_IN_REQUEST_CS_STATE);
    int comp_with_received = lamport_key_compare(&current_req_time, &received_req_time);
    // if process not in request cs state of if req time older than received req time
    if (comp_with_not_in_req_state == 0 || comp_with_received == 1) {
        account_reply_cs(account, from);
    } else {
        account->delayed_replies.arr[from] = 1;
    }
}

void account_request_cs(account_node *account) {
    timestamp_t timestamp = inc_lamport_time();
    const lamport_key req_time = {.id = account->node.id, .time = timestamp};
    account->current_request_time = req_time;
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = CS_REQUEST,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    send_multicast(&(account->node), &msg); // broadcast request
    account_loop_start(account); // block on receiving messages until we can enter to cs
}


void account_release_cs(account_node *account) {
    account->state_flags.reply_received = 0;
    account->current_request_time = NOT_IN_REQUEST_CS_STATE;
    timestamp_t timestamp = inc_lamport_time();
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = CS_REPLY,
            .s_payload_len = 0,
            .s_local_time = timestamp,
            .s_magic = MESSAGE_MAGIC};
    const local_id max_id = (local_id) (account->delayed_replies.sz - 1);
    // send delayed replies
    for(local_id id = 0; id <= max_id; ++id) {
        if (id != PARENT_ID && id != account->node.id && account->delayed_replies.arr[id]) {
            while (send(&(account->node), id, &msg) != 0);
            account->delayed_replies.arr[id] = 0;
        }
    }

}

void account_start_phase(account_node *account) {
    account_send_start_to_all(account);
    account_loop_start(account); // block on receiving messages until received all start messages
}

void account_done_phase(account_node *account) {
    account_send_done_to_all(account);
    account->state_flags.work_done_flag = true;
    account_loop_start(account);// block on receiving messages until received all done messages
}
