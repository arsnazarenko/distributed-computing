#include <stdio.h>
#include "pipes_util.h"

void close_fd(int fd) {
    if (fd > 2) {
        if (close(fd) != 0) { perror("close() failed"); }
    }
}

void close_pipe(half_duplex_pipe hdp) {
    close_fd(hdp.io.fd_read);
    close_fd(hdp.io.fd_write);
}
