#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "program_arg.h"
#include "context.h"
#include "ipc.h"
#include "logger.h"
#include "node.h"
#include "pa1.h"

// debug methods
//static void context_debug(const context *ctx) {
//    printf("Context: sz: %zu\n", ctx->sz);
//    for (size_t i = 0; i < ctx->sz; ++i) {
//        for (size_t j = 0; j < ctx->sz; ++j) {
//            printf("[%3d,%3d,%3d,%3d] ",
//                   ctx->pipe_table[i][j].input_pipe.io.fd_read,
//                   ctx->pipe_table[i][j].input_pipe.io.fd_write,
//                   ctx->pipe_table[i][j].output_pipe.io.fd_read,
//                   ctx->pipe_table[i][j].output_pipe.io.fd_write);
//        }
//        printf("\n");
//    }
//}
//
//static void node_debug(const node *node) {
//    printf("id: %d, list: %zu, { ", node->id, node->neighbours.sz);
//    for (size_t i = 0; i < node->neighbours.sz; ++i) {
//        printf("(%d, %d) ",
//               node->neighbours.interfaces[i].fd_read,
//               node->neighbours.interfaces[i].fd_write);
//    }
//    printf("}\n");
//}
//
//static void send_debug(const node *node, Message *msg) {
//    printf("Process %d send to all test_msg: { type: %d, p_len: %d, time: %d, magic: %d }\n",
//           node->id,
//           (*msg).s_header.s_type,
//           (*msg).s_header.s_payload_len,
//           (*msg).s_header.s_local_time,
//           (*msg).s_header.s_magic);
//}
//
//static void received_debug(const node *node, local_id from
//        , Message *msg) {
//    printf("process %d received from %d test_msg: { type: %d, p_len: %d, time: %d, magic: %d }\n",
//           node->id,
//           from,
//           (*msg).s_header.s_type,
//           (*msg).s_header.s_payload_len,
//           (*msg).s_header.s_local_time,
//           (*msg).s_header.s_magic);
//}


static void send_msg(node *node, MessageType type) {
    Message msg;
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

/**
 * todo: change function with if, else chain
 * todo: add some struct for phase with fields:
 * todo: phase_start_callback, next_phase_condition, phase_end_callback
**/
static void phase(node *node, MessageType type) {
    local_id max_id = (local_id) (node->neighbours.sz - 1);
    size_t process_count = node->neighbours.sz;
    size_t received = 0;
    for (local_id id = 1; id <= max_id; ++id) {
        if (node->id == id) {
            send_msg(node, type);

        } else {
            Message msg;
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

//fixme: parse args with getopt.h
int main(int argc, char **argv) {
    arguments program_arg;
    parse_program_args(argc, argv, &program_arg);
    logger_create();
    context context;
    if (context_create(&context, program_arg.child_proc_number + 1) != 0) {
        exit(1);
    }
    local_id max_child_id = (local_id) program_arg.child_proc_number;
    for (local_id child_id = 1; child_id <= max_child_id; ++child_id) {
        int pid = fork();
        if (pid == -1) {
            perror("fork() failed");
            context_destroy(&context);
            exit(1);
        } else if (pid == 0) {
            node node;
            node_create(&node, child_id, &context);
            context_destroy(&context);
            phase(&node, STARTED);
            phase(&node, DONE);
            node_destroy(&node);
            logger_destroy();
            exit(0);
        }
    }
    node node;
    node_create(&node, PARENT_ID, &context);
    context_destroy(&context);
    phase(&node, STARTED);
    phase(&node, DONE);
    node_destroy(&node);
    int status;
    for (size_t i = 0; i < program_arg.child_proc_number; ++i) {
        wait(&status);
    }
    logger_destroy();
    return 0;
}
