#ifndef LAB1_PIPES_UTIL_H
#define LAB1_PIPES_UTIL_H
// max number of processes in program
#define N_PROC 10
#include <unistd.h>

typedef union half_duplex_pipe {
    int fds[2];
    struct io {
        int fd_read;
        int fd_write;
    } io;
} half_duplex_pipe;

typedef struct duplex_pipe {
    half_duplex_pipe input_pipe;
    half_duplex_pipe output_pipe;
} duplex_pipe;

typedef struct node_interface {
    int fd_read;
    int fd_write;
} node_interface;

typedef struct adjacent_list {
    size_t sz;
    node_interface interfaces[N_PROC];
} adjacent_list;

void close_fd(int fd);

void close_pipe(half_duplex_pipe hdp);

#endif //LAB1_PIPES_UTIL_H
