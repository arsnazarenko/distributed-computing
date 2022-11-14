#ifndef LAB1_PIPES_UTIL_H
#define LAB1_PIPES_UTIL_H
#include <unistd.h>


enum {
    FD_READ = 0,
    FD_WRITE = 1
};

typedef struct duplex_pipe {
    int input_pipe[2];
    int output_pipe[2];
} duplex_pipe;

void close_fd(int fd);

void close_pipe(int pipe[2]);

#endif //LAB1_PIPES_UTIL_H
