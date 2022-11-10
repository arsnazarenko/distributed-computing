#include <errno.h>
#include <string.h>
#include "client_node.h"
#include "logger.h"

typedef void (*all_msg_handler)(client_node *account);
typedef void (*msg_handler)(client_node *account, Message *message);

static void client_handle_all_start(client_node *client);
static void client_handle_all_done(client_node *client);
static void client_handle_history(client_node *client, Message *message);

static void client_wait_all_msg(client_node *client, MessageType type, msg_handler on_receive, all_msg_handler on_receive_all);
static void client_send_stop_to_all(client_node *client);

void client_create(client_node *client, local_id id, context *ctx) {
    node_create(&(client->parent_node), id, ctx);
    client->all_history.s_history_len = 0;
    memset(&(client->all_history.s_history[0]), 0, sizeof(client->all_history.s_history));
}

void client_destroy(client_node *client) {
    node_destroy(&(client->parent_node));
}

static void client_wait_all_msg(client_node *client, MessageType type, msg_handler on_receive, all_msg_handler on_receive_all) {
    const size_t account_node_number = client->parent_node.neighbours.sz - 1;
    Message msg;
    size_t received = 0;
    while(received != account_node_number) {
        int res = receive_any(&(client->parent_node), &msg);
        if (res == -1 && errno == EAGAIN) { continue; }
        if (type == (MessageType) msg.s_header.s_type) {
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

static void client_handle_all_start(client_node *client) {
    log_received_all_started(client->parent_node.id);
}

static void client_handle_all_done(client_node *client) {
    log_received_all_done(client->parent_node.id);
}

static void client_handle_history(client_node *client, Message *message) {
    BalanceHistory *received_history = (BalanceHistory*) message->s_payload;
    BalanceHistory *ptr = &(client->all_history.s_history[received_history->s_id - 1]);
    memcpy(ptr, received_history, sizeof(BalanceHistory));
    ++(client->all_history.s_history_len);
}

static void client_send_stop_to_all(client_node *client) {
    Message msg;
    msg.s_header = (MessageHeader) {
            .s_type = STOP,
            .s_payload_len = 0,
            .s_local_time = get_physical_time(),
            .s_magic = MESSAGE_MAGIC
    };
    send_multicast(&(client->parent_node), &msg);
}


void client_transfer(client_node *client, local_id src, local_id dst, balance_t amount) {
    Message msg;
    memset(msg.s_payload, 0, sizeof(TransferOrder));

    TransferOrder *transferOrder = (TransferOrder*) msg.s_payload;
    transferOrder->s_src = src;
    transferOrder->s_dst = dst;
    transferOrder->s_amount = amount;

    msg.s_header = (MessageHeader) {
            .s_type = TRANSFER,
            .s_payload_len = sizeof(TransferOrder),
            .s_local_time = get_physical_time(),
            .s_magic = MESSAGE_MAGIC
    };

    while (send(&(client->parent_node), src, &msg) != 0);
    while( receive(&(client->parent_node), dst, &msg) != 0);
}
void client_first_phase(client_node *client) {
    client_wait_all_msg(client, STARTED, NULL, client_handle_all_start);
}
void client_third_phase(client_node *client) {
    client_send_stop_to_all(client);
    client_wait_all_msg(client, DONE, NULL, client_handle_all_done);
    client_wait_all_msg(client, BALANCE_HISTORY, client_handle_history, NULL);
}
