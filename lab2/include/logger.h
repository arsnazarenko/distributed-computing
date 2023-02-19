#ifndef LAB1_LOGGER_H
#define LAB1_LOGGER_H

#include <stdint.h>
#include <stdio.h>
#include "banking.h"
#include "ipc.h"
#include "pipe_utils.h"

int logger_create(void);
void logger_destroy(void);

void log_started(local_id id, int pid, int ppid, balance_t balance);
void log_received_all_started(local_id id);
void log_done(local_id id, balance_t balance);
void log_received_all_done(local_id id);
void log_transfer_in(local_id src_id, balance_t amount, local_id dst_id);
void log_transfer_out(local_id src_id, balance_t amount, local_id dst_id);

void log_pipe_open(int pipe[2]);

#endif //LAB1_LOGGER_H
