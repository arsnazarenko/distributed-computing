#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arg_utils.h"


static const char * const SHORT_OPTIONS = "hp:";

static const char * const USAGE = "Usage: ./pa3 -p [N] [START_BALANCE]...\n\n"
                                  "Options:\n"
                                  "%-40s%s\n"
                                  "%-40s%s\n";

static const char * const TRY_HELP = "Try ./pa3 --help for more information.\n";

static const struct option LONG_OPTIONS[] = {
        {"help",           no_argument,       NULL, 'h'},
        {"process-number", required_argument, NULL, 'p'},
        {NULL, 0,                             NULL, 0}
};

static void usage(void) {
    printf(
            USAGE,
            "-h, --help",
            "Display this help text and exit",
            "-p [N], --process-number[=N]",
            "Required option. Define number of child processes created in the program. Value can be between 1 and 9 inclusive."
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
            case '?':
            default: {
                try();
                exit(-1);
            }
        }
    }
}

static void parse_non_option_args(int argc, char** argv, arguments* arg) {
    memset(arg->start_balances, 0, sizeof(arg->start_balances));
    size_t non_option_arg_count = argc - optind;
    if (non_option_arg_count < arg->child_proc_number) {
        fputs("Starting balances must be specified for each account.\n", stderr);
        exit(-1);
    }
    size_t balance_idx = 1;
    for (int index = optind; index < argc; ++index) {
        size_t start_balance = strtol(argv[index], NULL, 10);
        if (start_balance < 1 || start_balance > MAX_BALANCE) {
            fprintf(stderr, "Value of starting balance can be between 1 and 99 inclusive\n"
                    "Starting balance of account with id %zu: %zu\n", balance_idx, start_balance);
            exit(-1);
        }
        arg->start_balances[balance_idx++] = (balance_t) start_balance;
    }
}

void parse_program_args(int argc, char** argv, arguments* arg) {

    parse_option_args(argc, argv, arg);
    validate_option_args(arg);
    parse_non_option_args(argc, argv, arg);
}


