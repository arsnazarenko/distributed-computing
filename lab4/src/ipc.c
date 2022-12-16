#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ipc.h"
#include "node.h"
#include "lamport_time.h"

static const size_t MAX_RETRY_NUMBER = 3;

static int read_with_retry(int fd_from, void *buf, ssize_t sz) {
    size_t retry = 0;
    const ssize_t len = sz;
    ssize_t was_read = 0;
    ssize_t must_read = len - was_read;
    char *b_buf = (char *) buf;
    while ((was_read += read(fd_from,
                             b_buf + was_read,
                             len - was_read)) != must_read) {
        if (was_read > 0 && retry < MAX_RETRY_NUMBER) {
            must_read = len - was_read;
            ++retry;
        } else {
            if (errno != EAGAIN) {
                perror("read() failed");
            }
            return -1;
        }
    }
    return 0;
}

static int write_with_retry(int fd_dst, const void *buf, ssize_t sz) {
    size_t retry = 0;
    const ssize_t len = sz;
    ssize_t was_write = 0;
    ssize_t must_write = len - was_write;
    char *b_buf = (char *) buf;
    while ((was_write += write(fd_dst,
                               b_buf + was_write,
                               len - was_write)) != must_write) {
        if (was_write > 0 && retry < MAX_RETRY_NUMBER) {
            must_write = len - was_write;
            ++retry;
        } else {
            if (errno != EAGAIN) {
                perror("write() failed");
            }
            return -1;
        }
    }
    return 0;
}

int send(void *self, local_id dst, const Message *msg) {
    const node *self_node = (node *) self;
    if (dst == self_node->id) { return -1; }
    return write_with_retry(self_node->neighbours.interfaces[dst].fd_write,
                            msg,
                            (ssize_t) (sizeof(MessageHeader) + msg->s_header.s_payload_len));
}

int send_multicast(void *self, const Message *msg) {
    const node *self_node = (node *) self;
    const local_id max_id = (local_id) (self_node->neighbours.sz - 1);
    for (local_id id = 0; id <= max_id; ++id) {
        if (id != self_node->id) {
            if (send(self, id, msg) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

int receive(void *self, local_id from, Message *msg) {
    const node *self_node = (node *) self;
    if (from == self_node->id) { return -1; }
    if (read_with_retry(self_node->neighbours.interfaces[from].fd_read,
                        &(msg->s_header),
                        (ssize_t) sizeof(MessageHeader)) == -1) {
        return -1;
    }
    return read_with_retry(self_node->neighbours.interfaces[from].fd_read,
                           msg->s_payload,
                           msg->s_header.s_payload_len);
}

int receive_any(void *self, Message *msg) {
    const node *self_node = (node *) self;
    const local_id max_id = (local_id) (self_node->neighbours.sz - 1);
    for (local_id id = 0; id <= max_id; ++id) {
        if (id != self_node->id) {
            if (receive(self, id, msg) != -1) { return 0; }
        }
    }
    return -1;
}
