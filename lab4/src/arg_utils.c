#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arg_utils.h"


static const char * const SHORT_OPTIONS = "hmp:";

static const char * const USAGE = "Usage: ./pa4 -p [N]\n\n"
                                  "Options:\n"
                                  "%-40s%s\n"
                                  "%-40s%s\n"
                                  "%-40s%s\n";

static const char * const TRY_HELP = "Try ./pa4 --help for more information.\n";

static const struct option LONG_OPTIONS[] = {
        {"help",           no_argument,       NULL, 'h'},
        {"process-number", required_argument, NULL, 'p'},
        {"mutexl", no_argument, NULL, 'm'},
        {NULL, 0,                             NULL, 0}
};

static void usage(void) {
    printf(
            USAGE,
            "-h, --help",
            "Display this help text and exit",
            "-p [N], --process-number[=N]",
            "Required option. Define number of child processes created in the program. Value can be between 1 and 9 inclusive.",
            "-m, --mutexl",
            "Using the Lamport mutual exclusion algorithm"
    );
}

static void try(void) {
    fputs(TRY_HELP, stderr);
}

static void validate_option_args(const arguments* arg) {
    if (arg->child_proc_number < 1 || arg->child_proc_number > MAX_PROC_ID) {
        fprintf(stderr,
                "Invalid value of required option -%c [--%s]\n",
                LONG_OPTIONS[1].val,
                LONG_OPTIONS[1].name);
        try();
        exit(-1);
    }
}

void parse_option_args(int argc, char** argv, arguments* arg) {
    if (argc < 2) {
        usage();
        exit(-1);
    }
    int rez = -1;
    int option_index = -1;
    while ((rez = getopt_long(argc,
                              argv,
                              SHORT_OPTIONS,
                              LONG_OPTIONS,
                              &option_index)) != -1) {
        switch (rez) {
            case 'h': {
                usage();
                exit(0);
            }
            case 'p': {
                arg->child_proc_number = strtol(optarg, NULL, 10);
                break;
            }
            case 'm': {
                arg->mutex_flag = true;
                break;
            }
            case '?':
            default: {
                try();
                exit(-1);
            }
        }
    }
}


void parse_program_args(int argc, char** argv, arguments* arg) {
    arg->child_proc_number = 0;
    arg->mutex_flag = false;

    parse_option_args(argc, argv, arg);
    validate_option_args(arg);
}


