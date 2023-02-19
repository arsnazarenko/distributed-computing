#include <assert.h>
#include "common.h"
#include "logger.h"
#include "pa2345.h"

static FILE* pipes_log_file = NULL;
static FILE* events_log_file = NULL;

static const char * const log_pipe_open_fmt = "Pipe(%3d,%3d) was opened\n";

static void close_file(FILE* file) {
    if (file != NULL) {
        if (fclose(file) != 0) {
            perror("fclose() failed");
        }
    }
}

void logger_destroy(void) {
    close_file(pipes_log_file);
    close_file(events_log_file);
}

int logger_create(void) {
    if ((pipes_log_file = fopen(pipes_log, "w")) == 0 ||
        (events_log_file = fopen(events_log, "w")) == 0) {
        perror("fopen() failed");
        // if some files was opened
        logger_destroy();
        return -1;
    }
    return 0;
}

void log_started(local_id id, int pid, int ppid, balance_t balance) {
    assert(events_log_file != NULL);
    printf(log_started_fmt, get_lamport_time(), id, pid, ppid, balance);
    fprintf(events_log_file, log_started_fmt, get_lamport_time(), id, pid, ppid, balance);
    fflush(stdout);
    fflush(events_log_file);
}

void log_received_all_started(local_id id) {
    assert(events_log_file != NULL);
    printf(log_received_all_started_fmt, get_lamport_time(), id);
    fprintf(events_log_file, log_received_all_started_fmt, get_lamport_time(), id);
    fflush(stdout);
    fflush(events_log_file);
}

void log_done(local_id id, balance_t balance) {
    assert(events_log_file != NULL);
    printf(log_done_fmt, get_lamport_time(), id, balance);
    fprintf(events_log_file, log_done_fmt, get_lamport_time(), id, balance);
    fflush(stdout);
    fflush(events_log_file);
}

void log_received_all_done(local_id id) {
    assert(events_log_file != NULL);
    printf(log_received_all_done_fmt, get_lamport_time(), id);
    fprintf(events_log_file, log_received_all_done_fmt, get_lamport_time(), id);
    fflush(stdout);
    fflush(events_log_file);
}

void log_pipe_open(int pipe[2]) {
    assert(pipes_log_file != NULL);
    fprintf(pipes_log_file, log_pipe_open_fmt, pipe[FD_READ], pipe[FD_WRITE]);
    fflush(pipes_log_file);
}

void log_transfer_in(local_id src_id, balance_t amount, local_id dst_id) {
    assert(events_log_file != NULL);
    printf(log_transfer_in_fmt, get_lamport_time(), dst_id, amount, src_id);
    fprintf(events_log_file, log_transfer_in_fmt, get_lamport_time(), src_id, amount, dst_id);
    fflush(stdout);
    fflush(events_log_file);
}

void log_transfer_out(local_id src_id, balance_t amount, local_id dst_id) {
    assert(events_log_file != NULL);
    printf(log_transfer_out_fmt, get_lamport_time(), src_id, amount, dst_id);
    fprintf(events_log_file, log_transfer_out_fmt, get_lamport_time(), src_id, amount, dst_id);
    fflush(stdout);
    fflush(events_log_file);
}
