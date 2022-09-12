#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ipc.h"

typedef struct {
    int fd_int;
    int fd_out;
} node_pipes;



int main(int argc, char **argv) {

    int pipes[11][11][2] = {0};

    for (size_t i = 0; i < 11; ++i) {
        for (size_t j = 0; j < 11; ++j) {
            if (i == j) { continue; }
            if (pipe(pipes[i][j]) != 0) {
                perror("pipe() failed");
                break;
            }
            printf("%zu.%zu) fds = {%d, %d}\n", i, j, pipes[i][j][0], pipes[i][j][0]);
        }
    }

    for (size_t i = 0; i < 11; ++i) {
        for (size_t j = 0; j < 11; ++j) {
            if (i == j) { continue; }
            if (pipes[i][j][0] != 0) {
                close(pipes[i][j][0]);
            }
            if (pipes[i][j][1] != 0) {
                close(pipes[i][j][1]);
            }
        }
    }

    /**
     * Duplex pipes between parent and child:
     * https://stackoverflow.com/questions/60104813/using-two-pipes-to-communicate-between-parent-process-and-child-process
     *
     */

    if (argc != 3) {
        fputs("Wrong number of arguments\n", stderr);
        return -1;
    }
    const uint16_t n_proc = strtol(argv[2], NULL, 10);
    const char *const option = argv[1];
    if (strcmp("-p", option) != 0) {
        fputs("Wrong option", stderr);
    }
    if (n_proc <= 0 || n_proc > 10) {
        fputs("Invalid value of option \"-p\"", stderr);
        return -1;
    }
    local_id proc_id = 1;
    for (size_t i = 0; i < n_proc; ++i, ++proc_id) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork() failed");
            exit(1);
        } else if (pid == 0) {
            // child
            printf("CHILD: pid = %d, local_id = %d\n", getpid(), proc_id);
            // fixme: some_child_function();
            exit(0);
        }
    }
    // parent
    printf("PARENT: pid = %d, local_id = %d\n", getpid(), 0);
    int status;
    pid_t pid;
    for (size_t i = 0; i < n_proc; ++i) {
        printf("%zu. ", i);
        pid = wait(&status);
        printf("Child with pid %d exit with status %d\n", pid, status);
    }
    return 0;
}

