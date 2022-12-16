#ifndef LAB1_INPUT_ARGS_H
#define LAB1_INPUT_ARGS_H
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

enum {
    MAX_PROC_ID = 9,
    MAX_BALANCE = 99
};

typedef struct arguments {
    size_t child_proc_number;
    bool mutex_flag;
} arguments;

void parse_program_args(int argc, char** argv, arguments* arg);

#endif //LAB1_INPUT_ARGS_H
