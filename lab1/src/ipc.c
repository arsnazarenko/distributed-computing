#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "ipc.h"
#include "node.h"

int send(void *self, local_id dst, const Message *msg) {
    const node *self_node = (node *) self;
    if (dst == self_node->id) { return -1; }
    if (write(self_node->neighbours.interfaces[dst].fd_write,
              msg, sizeof(MessageHeader) + strlen(msg->s_payload)) == -1) {
        perror("write() failed");
        return -1;
    }
    return 0;
}

int send_multicast(void *self, const Message *msg) {
    const node *self_node = (node *) self;
    const local_id max_id = (local_id) (self_node->neighbours.sz - 1);
    for (local_id id = 0; id <= max_id; ++id) {
        if (id == self_node->id) { continue; }
        if (send(self, id, msg) != 0) {
            return -1;
        }
    }
    return 0;
}

int receive(void *self, local_id from, Message *msg) {
    const node *self_node = (node *) self;
    if (from == self_node->id) { return -1; }
    if (read(self_node->neighbours.interfaces[from].fd_read,
             msg, MAX_MESSAGE_LEN) == -1) {
        perror("read() failed");
        return -1;
    }
    return 0;
}

int receive_any(void *self, Message *msg) {
    (void) self;
    (void) msg;
    return -1;
}
