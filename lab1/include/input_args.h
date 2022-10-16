#ifndef LAB1_INPUT_ARGS_H
#define LAB1_INPUT_ARGS_H
#include <stdio.h>

typedef struct arguments {
    size_t child_proc_number;
    // other args
} arguments;

void parse_arguments(int argc, char** argv, arguments* arg);
void validate_arguments(const arguments* arg);

#endif //LAB1_INPUT_ARGS_H
