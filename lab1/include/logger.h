#ifndef LAB1_LOGGER_H
#define LAB1_LOGGER_H

#include <stdint.h>
#include "context.h"

extern int pipes_log_fd;
extern int events_log_fd;

int logger_create(void);
void logger_destroy(void);

void log_started(int8_t id, int pid, int ppid);
void log_received_all_started(int8_t id);
void log_done(int8_t id);
void log_received_all_done(int8_t id);

void log_pipe_open(half_duplex_pipe pipe);

#endif //LAB1_LOGGER_H
