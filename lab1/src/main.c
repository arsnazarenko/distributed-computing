#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "context.h"
#include "ipc.h"
#include "logger.h"
#include "node.h"
#include "pa1.h"

static void context_debug(const context *ctx) {
    printf("Context: sz: %zu\n", ctx->sz);
    for (size_t i = 0; i < ctx->sz; ++i) {
        for (size_t j = 0; j < ctx->sz; ++j) {
            printf("[%3d,%3d,%3d,%3d] ",
                   ctx->pipe_table[i][j].input_pipe.fd_read,
                   ctx->pipe_table[i][j].input_pipe.fd_write,
                   ctx->pipe_table[i][j].output_pipe.fd_read,
                   ctx->pipe_table[i][j].output_pipe.fd_write);
        }
        printf("\n");
    }
}

static void node_debug(const node *node) {
    printf("id: %d, list: %zu, { ", node->id, node->neighbours.sz);
    for (size_t i = 0; i < node->neighbours.sz; ++i) {
        printf("(%d, %d) ",
               node->neighbours.interfaces[i].fd_read,
               node->neighbours.interfaces[i].fd_write);
    }
    printf("}\n");
}

static void send_debug(const node *node, Message *msg) {
    printf("Process %d send to all test_msg: { type: %d, p_len: %d, time: %d, magic: %d }\n",
           node->id,
           (*msg).s_header.s_type,
           (*msg).s_header.s_payload_len,
           (*msg).s_header.s_local_time,
           (*msg).s_header.s_magic);
}

static void received_debug(const node *node, local_id from
        , Message *msg) {
    printf("process %d received from %d test_msg: { type: %d, p_len: %d, time: %d, magic: %d }\n",
           node->id,
           from,
           (*msg).s_header.s_type,
           (*msg).s_header.s_payload_len,
           (*msg).s_header.s_local_time,
           (*msg).s_header.s_magic);
}

static void send_msg(node* node, MessageType type) {
    Message msg = {0};
    if (type == STARTED) {
        log_started(node->id, getpid(), getppid());
        sprintf(msg.s_payload, log_started_fmt, node->id, getpid(), getppid());
    } else if (type == DONE) {
        log_done(node->id);
        sprintf(msg.s_payload, log_done_fmt, node->id);
    }
    // else other types
    msg.s_header = (MessageHeader)
            {.s_type = type,
             .s_payload_len = strlen(msg.s_payload),
             .s_local_time = 0,
             .s_magic = MESSAGE_MAGIC
            };
    send_multicast(node, &msg);
}

static void phase(node* node, MessageType type) {
    local_id max_id = (local_id) (node->neighbours.sz - 1);
    size_t process_count = node->neighbours.sz;
    size_t received = 0;
    for (local_id id = 1; id <= max_id; ++id) {
        if (node->id == id) {
            send_msg(node, type);

        } else {
            Message msg = {0};
            receive(node, id, &msg);
            if (msg.s_header.s_type == (int16_t) type) {
                ++received;
            }
        }
    }
    if (node->id == PARENT_ID) {
      if (received == process_count - 1) {
          if (type == STARTED) { log_received_all_started(node->id); }
          else if (type == DONE) { log_received_all_done(node->id); }
          // else other types
      }
    } else {
        if (received == process_count - 2) {
            if (type == STARTED) { log_received_all_started(node->id); }
            else if (type == DONE) { log_received_all_done(node->id); }
            // else other types
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fputs("Wrong number of arguments\n", stderr);
        return -1;
    }
    const char *const option = argv[1];
    if (strcmp("-p", option) != 0) {
        fputs("Wrong option\n", stderr);
        return -1;
    }
    const size_t child_proc_number = strtol(argv[2], NULL, 10);
    if (child_proc_number < 1 || child_proc_number > 9) {
        fputs("Invalid value of option \"-p\"\n", stderr);
        return -1;
    }
    logger_create();
    context context = {0};
    if (context_create(&context, child_proc_number + 1) != 0) {
        exit(1);
    }
    local_id max_child_id = (local_id) child_proc_number;
    for (local_id child_id = 1; child_id <= max_child_id; ++child_id) {
        pid_t pid = fork();
        if (pid == -1) {
            // error
            perror("fork() failed");
            context_destroy(&context);
            exit(1);
        } else if (pid == 0) {
            // child
            node node = {0};
            node_create(&node, child_id, &context);
            context_destroy(&context);
            phase(&node, STARTED);
            phase(&node, DONE);
            node_destroy(&node);
            logger_destroy();
            exit(0);
        }
    }
    node node = {0};
    node_create(&node, PARENT_ID, &context);
    context_destroy(&context);
    phase(&node, STARTED);
    phase(&node, DONE);
    node_destroy(&node);
    int status;
    pid_t pid;
    for (size_t i = 0; i < child_proc_number; ++i) {
        pid = wait(&status);
    }
    logger_destroy();
    return 0;
}




