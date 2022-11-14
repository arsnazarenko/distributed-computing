#include "lamport_time.h"

static timestamp_t local_timestamp = 0;

static timestamp_t max(timestamp_t a, timestamp_t b) {
    return (a > b) ? a : b;
}


timestamp_t get_lamport_time(void) {
    return local_timestamp;
}

timestamp_t inc_lamport_time(void) {
    return ++local_timestamp;
}

timestamp_t sync_lamport_time(timestamp_t received) {
    local_timestamp = max(local_timestamp, received);
    return local_timestamp;
}
