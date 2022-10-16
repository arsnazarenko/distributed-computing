#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "input_args.h"

static const char * const short_options = "hp:";
static const struct option long_options[] = {
        {"help",           no_argument,       NULL, 'h'},
        {"process-number", required_argument, NULL, 'p'},
        {NULL, 0,                             NULL, 0}
};

static void usage(void) {
    printf(
            "Usage: ./main -p [N]\n\n"
            "Options:\n"
            "%-40s%s\n"
            "%-40s%s\n",
            "-h, --help", "Display this help text and exit",
            "-p [N], --process-number[=N]",
            "Required option. Define number of child processes created in the program. Value can be between 1 and 9 inclusive"
    );
}

static void try(void) {
    fputs("Try ./main --help for more information.\n", stderr);

}

void parse_arguments(int argc, char** argv, arguments* arg) {
    if (argc < 2) {
        usage();
        exit(-1);
    }
    arg->child_proc_number = 0;
    int rez = -1;
    int option_index = -1;
    while ((rez = getopt_long(argc,
                              argv,
                              short_options,
                              long_options,
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

void validate_arguments(const arguments* arg) {
    if (arg->child_proc_number < 1 || arg->child_proc_number > 9) {
        fprintf(stderr,
                "Invalid value of required option -%c [--%s]\n",
                long_options[1].val,
                long_options[1].name);
        try();
        exit(-1);
    }
}

