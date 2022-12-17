#ifndef LAB3_LAMPORT_TIME_H
#define LAB3_LAMPORT_TIME_H
#include "ipc.h"

timestamp_t sync_lamport_time(timestamp_t received);
timestamp_t inc_lamport_time(void);

#endif //LAB3_LAMPORT_TIME_H
