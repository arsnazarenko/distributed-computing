#ifndef LAB3_LAMPORT_TIME_H
#define LAB3_LAMPORT_TIME_H
#include "ipc.h"

typedef struct lamport_key {
    timestamp_t time;
    local_id id;
} lamport_key;

int lamport_key_compare(const lamport_key* lhs, const lamport_key* rhs);

timestamp_t sync_lamport_time(timestamp_t received);
timestamp_t inc_lamport_time(void);

#endif //LAB3_LAMPORT_TIME_H
