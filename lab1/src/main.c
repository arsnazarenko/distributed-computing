#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "context.h"
#include "node.h"
#include "ipc.h"
#include "pa1.h"

/**
 * Duplex pipes between parent and child:
 * https://stackoverflow.com/questions/60104813/using-two-pipes-to-communicate-between-parent-process-and-child-process
 *
 */

int main(int argc, char **argv) {
    if (argc != 3) {
        fputs("Wrong number of arguments\n", stderr);
        return -1;
    }
    const char * const option = argv[1];
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
            printf("CHILD: pid = %d, local_id = %d\n", getpid(), proc_id);
            node node = {0};
            node_create(&node, proc_id, &context);
            context_destroy(&context);
            // fixme: child work
            node_destroy(&node);
            exit(0);
        }
    }
    printf("PARENT: pid = %d, local_id = %d\n", getpid(), 0);
    node parent_node = {0};
    node_create(&parent_node, 0, &context);
    context_destroy(&context);
    node_destroy(&parent_node);
    int status;
    pid_t pid;
    for (size_t i = 0; i < n_proc; ++i) {
        printf("%zu. ", i + 1);
        pid = wait(&status);
        printf("Child with pid %d exit with status %d\n", pid, status);
    }
    return 0;
}


