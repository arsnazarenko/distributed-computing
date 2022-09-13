#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include "ipc.h"
#include "common.h"
#include "pa1.h"

/**
 * Duplex pipes between parent and child:
 * https://stackoverflow.com/questions/60104813/using-two-pipes-to-communicate-between-parent-process-and-child-process
 *
 */

#define MAX_PROC_COUNT 11

typedef union {
    int fds[2];
    struct {
        int fd_in;
        int fd_out;
    };
} half_duplex_pipe;

typedef struct {
    half_duplex_pipe input_pipe;
    half_duplex_pipe output_pipe;
} duplex_pipe;

typedef struct {
    int fd_in;
    int fd_out;
} node_interface;

typedef struct {
    local_id id;
    node_interface adjacent[11];
} node;


static void close_pipe(half_duplex_pipe pipe) {
    if (pipe.fd_in > 2) { close(pipe.fd_in); }
    if (pipe.fd_out > 2) { close(pipe.fd_out); }
}

static void clear_pipe_table(duplex_pipe **pipe_table, const uint16_t sz) {
    assert(sz > 0);
    for (size_t i = 0; i < sz + 1; ++i) {
        for (size_t j = 0; j < sz; ++j) {
            if (i == j) { continue; }
            close_pipe(pipe_table[i][j].input_pipe);
            close_pipe(pipe_table[i][j].output_pipe);
        }
    }
}

static void init_pipe_table(duplex_pipe **pipe_table, const uint16_t sz) {
    assert(sz > 0);
    for (uint16_t i = 0; i < sz + 1; ++i) {
        for (size_t j = 0; j < sz; ++j) {
            if (i == j) { continue; }
            half_duplex_pipe input_p = {0};
            half_duplex_pipe output_p = {0};
            if (pipe(input_p.fds) != 0 || pipe(output_p.fds) != 0) {
                perror("pipe() failed");
                clear_pipe_table(pipe_table, sz);
                exit(1);
            }
            pipe_table[i][j] = (duplex_pipe) {input_p, output_p};
            pipe_table[j][i] = (duplex_pipe) {output_p, input_p};
        }
    }
}

int main(int argc, char **argv) {

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
    const duplex_pipe pipes[MAX_PROC_COUNT][MAX_PROC_COUNT] = {0};
    init_pipe_table((duplex_pipe **) pipes, n_proc);

    local_id proc_id = 1;
    for (uint16_t i = 0; i < n_proc; ++i, ++proc_id) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork() failed");
            clear_pipe_table((duplex_pipe **) pipes, n_proc);
            exit(1);
        } else if (pid == 0) {
            // child
            printf("CHILD: pid = %d, local_id = %d\n", getpid(), proc_id);
            duplex_pipe *adjacent = (duplex_pipe *) pipes[proc_id];
            for (uint16_t k = 0; k < n_proc + 1; ++k) {
                if (k == proc_id) { continue; }
                close(adjacent[k].input_pipe.fd_in);
                close(adjacent[k].output_pipe.fd_out);
            }
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


