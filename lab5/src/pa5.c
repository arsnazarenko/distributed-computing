#include <stdlib.h>
#include <wait.h>
#include "account_node.h"
#include "client_node.h"
#include "logger.h"
#include "pa2345.h"


int main(int argc, char *argv[]) {
    arguments program_arg;
    parse_program_args(argc, argv, &program_arg);

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
            account_recv_handler account_handlers[CS_RELEASE + 1] = {
                    [STARTED] = account_handle_started,
                    [DONE] = account_handle_done,
                    [CS_REQUEST] = account_handle_request,
                    [CS_REPLY] = account_handle_reply,
            };
            account_create(&account, id, &context, account_handlers);
            context_destroy(&context);


            char buf[100];
            int iter_number = id * 5;
            bool mutex = program_arg.mutex_flag;
            account_start_phase(&account);
            for (int i = 1; i <= iter_number; ++i) {
                sprintf(buf, log_loop_operation_fmt, id, i, iter_number);

                if (mutex) { account_request_cs(&account); } // MUTEX ACQUIRE
                                                                    //
                print(buf);                                       //  - CRITICAL  SECTION
                                                                    //
                if (mutex) { account_release_cs(&account); } //  MUTEX ACQUIRE
            }
            account_done_phase(&account);

            account_destroy(&account);
            logger_destroy();
            exit(0);
        }
    }
    client_node client;
    client_recv_handler client_handlers[CS_RELEASE + 1] = {
            [STARTED] = client_handle_started,
            [DONE] = client_handle_done,
    };
    client_create(&client, PARENT_ID, &context, client_handlers);
    context_destroy(&context);  // close unused pipes

    client_start_phase(&client);
    client_done_phase(&client);

    client_destroy(&client);

    int status;
    for (size_t i = 0; i < program_arg.child_proc_number; ++i) {
        wait(&status);
    }
    logger_destroy();
    return 0;
}
