#include <stdio.h>
#include "pipe_utils.h"

void close_fd(int fd) {
    if (fd > 2) {
        if (close(fd) != 0) { perror("close() failed"); }
    }
}

void close_pipe(int pipe[2]) {
    close_fd(pipe[FD_READ]);
    close_fd(pipe[FD_WRITE]);
}
