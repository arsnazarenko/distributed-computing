#include <stdlib.h>
#include <wait.h>
#include "account_node.h"
#include "client_node.h"
#include "logger.h"
#include "pa2345.h"
static void arg_dump(const arguments* args) {
    printf(
            "args {\n"
            "child_proc_number: %zu\n"
            "mutexl: %s\n"
            "}\n",
            args->child_proc_number,
            (args->mutex_flag ? "true" : "false")
            );
}

int main(int argc, char *argv[]) {
    arguments program_arg;
    parse_program_args(argc, argv, &program_arg);
    arg_dump(&program_arg);
    logger_create();
    context context;
    if (context_create(&context, program_arg.child_proc_number + 1) != 0) {
        exit(1);
    }
    local_id max_child_id = (local_id) program_arg.child_proc_number;
    for (local_id child_id = 1; child_id <= max_child_id; ++child_id) {
        int pid = fork();
        if (pid == -1) {
            perror("fork() failed");
            context_destroy(&context);
            exit(1);
        } else if (pid == 0) {
            local_id id = child_id;
            account_node account;
            account_create(&account, id, &context);
            context_destroy(&context);

            receive_handler account_handlers[CS_RELEASE + 1] = {
                    [STARTED] = account_handle_started,
                    [DONE] = account_handle_done,
                    [CS_REQUEST] = account_handle_request,
                    [CS_REPLY] = account_handle_reply,
                    [CS_RELEASE] = account_handle_release,
            };
            char buf[100];
            int iter_number = id * 5;

            account_first_phase(&account);
            for (int i = 1; i <= iter_number; ++i) {
                account_request_cs(&account);

                sprintf(buf, log_loop_operation_fmt, id, i, iter_number);
                print(buf);

                account_release_cs(&account);
            }
            account_third_phase(&account);

            account_destroy(&account);
            logger_destroy();
            exit(0);
        }
    }
    client_node client;
    client_create(&client, PARENT_ID, &context);
    context_destroy(&context);  // close unused pipes

    client_first_phase(&client);
    client_third_phase(&client);

    client_destroy(&client);

    int status;
    for (size_t i = 0; i < program_arg.child_proc_number; ++i) {
        wait(&status);
    }
    logger_destroy();
    return 0;
}