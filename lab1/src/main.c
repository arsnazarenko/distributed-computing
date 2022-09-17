#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "context.h"
#include "node.h"
#include "ipc.h"
#include "pa1.h"

//static void context_debug(const context *ctx) {
//    printf("Context: sz: %zu\n", ctx->sz);
//    for (size_t i = 0; i < ctx->sz; ++i) {
//        for (size_t j = 0; j < ctx->sz; ++j) {
//            printf("[%3d,%3d,%3d,%3d] ",
//                   ctx->pipe_table[i][j].input_pipe.fd_read,
//                   ctx->pipe_table[i][j].input_pipe.fd_write,
//                   ctx->pipe_table[i][j].output_pipe.fd_read,
//                   ctx->pipe_table[i][j].output_pipe.fd_write);
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


void work(node *node) {
    const local_id max_id = (local_id) (node->neighbours.sz - 1);
    local_id writer_id = 1;
    while (writer_id <= max_id) {
        if (node->id == writer_id) {
            Message msg = {0};
            sprintf(msg.s_payload, log_started_fmt, node->id, getpid(), getppid());
            msg.s_header = (MessageHeader)
                    {.s_type = STARTED,
                            .s_payload_len = strlen(msg.s_payload),
                            .s_local_time = 0,
                            .s_magic = MESSAGE_MAGIC
                    };
            send_multicast(node, &msg);
        } else {
            Message msg = {0};
            receive(node, writer_id, &msg);
            msg.s_payload[msg.s_header.s_payload_len] = 0; // add null-terminator for reading payload
            printf("Process %d (%d) received: %s", node->id, getpid(), msg.s_payload);
        }
        ++writer_id;
    }
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fputs("Wrong number of arguments\n", stderr);
        return -1;
    }
    const char *const option = argv[1];
    if (strcmp("-p", option) != 0) {
        fputs("Wrong option", stderr);
    }
    const size_t n_proc = strtol(argv[2], NULL, 10);
    if (n_proc <= 0 || n_proc > 10) {
        fputs("Invalid value of option \"-p\"", stderr);
        return -1;
    }
    context context = {0};
    if (!context_create(&context, n_proc + 1)) {
        context_destroy(&context);
        exit(1);
    }
    local_id proc_id = 1;
    for (size_t i = 0; i < n_proc; ++i, ++proc_id) {
        pid_t pid = fork();
        if (pid == -1) {
            // error
            perror("fork() failed");
            context_destroy(&context);
            exit(1);
        } else if (pid == 0) {
            // child
//            printf("CHILD: pid = %d, local_id = %d\n", getpid(), proc_id);
            node node = {0};
            node_create(&node, proc_id, &context);
            context_destroy(&context);
            work(&node);
            node_destroy(&node);
            exit(0);
        }
    }
//    printf("PARENT: pid = %d, local_id = %d\n", getpid(), 0);
    node node = {0};
    node_create(&node, 0, &context);
    context_destroy(&context);
    work(&node);
    node_destroy(&node);
    int status;
    pid_t pid;
    for (size_t i = 0; i < n_proc; ++i) {
        printf("%zu. ", i + 1);
        pid = wait(&status);
        printf("Child with pid %d exit with status %d\n", pid, status);
    }
    return 0;
}




