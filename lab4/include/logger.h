#ifndef LAB1_LOGGER_H
#define LAB1_LOGGER_H

#include <stdint.h>
#include <stdio.h>
#include "ipc.h"
#include "pipe_utils.h"

extern FILE* pipes_log_file;
extern FILE* events_log_file;

int logger_create(void);
void logger_destroy(void);

void log_started(local_id id, int pid, int ppid, balance_t balance);
void log_received_all_started(local_id id);
void log_done(local_id id, balance_t balance);
void log_received_all_done(local_id id);
void log_pipe_open(int pipe[2]);

#endif //LAB1_LOGGER_H
