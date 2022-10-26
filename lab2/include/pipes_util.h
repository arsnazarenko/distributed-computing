#ifndef LAB1_PIPES_UTIL_H
#define LAB1_PIPES_UTIL_H
#include <unistd.h>

#define N_PROC 10
#define FD_READ 0
#define FD_WRITE 1

typedef struct duplex_pipe {
    int input_pipe[2];
    int output_pipe[2];
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

void close_pipe(int pipe[2]);

#endif //LAB1_PIPES_UTIL_H
