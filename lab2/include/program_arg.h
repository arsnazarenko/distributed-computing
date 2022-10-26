#ifndef LAB1_INPUT_ARGS_H
#define LAB1_INPUT_ARGS_H
#include "unistd.h"
#include "stdint.h"


typedef struct arguments {
    size_t child_proc_number;
    uint8_t start_balances[10];
} arguments;

void parse_program_args(int argc, char** argv, arguments* arg);

#endif //LAB1_INPUT_ARGS_H
