#ifndef LAB1_LOGGER_H
#define LAB1_LOGGER_H

#include <stdint.h>
#include <stdio.h>
#include "ipc.h"
#include "pipes_util.h"

extern FILE* pipes_log_file;
extern FILE* events_log_file;

int logger_create(void);
void logger_destroy(void);

void log_started(local_id id, int pid, int ppid);
void log_received_all_started(local_id id);
void log_done(local_id id);
void log_received_all_done(local_id id);

void log_pipe_open(half_duplex_pipe);

#endif //LAB1_LOGGER_H
