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
int lamport_key_compare(const lamport_key* lhs, const lamport_key* rhs) {
    if (lhs->time > rhs->time) {
        return 1;
    } else if (lhs->time < rhs->time) {
        return -1;
    } else {
        if (lhs->id > rhs->id) { return 1;}
        else if (rhs->id > lhs->id) { return -1; }
        else { return 0; }
    }
}

