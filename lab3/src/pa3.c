#include <stdlib.h>
#include <wait.h>
#include <string.h>
#include <stdbool.h>
#include "account_node.h"
#include "client_node.h"
#include "logger.h"

void transfer(void *parent_data, local_id src, local_id dst,
              balance_t amount) {
    client_node *client_ptr = (client_node *) parent_data;
    client_transfer(client_ptr, src, dst, amount);
}

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
            account_create(&account, program_arg.start_balances[id], id, &context);
            context_destroy(&context);

            receive_handler account_handlers[CS_RELEASE + 1] = {
                    [STARTED] = account_handle_started,
                    [DONE] = account_handle_done,
                    [TRANSFER] = account_handle_transfer,
                    [STOP] = account_handle_stop
            };


            account_run(&account, account_handlers);
            account_destroy(&account);
            logger_destroy();
            exit(0);
        }
    }



    client_node client;
    client_create(&client, PARENT_ID, &context);
    context_destroy(&context);  // close unused pipes
    client_first_phase(&client);
    bank_robbery(&client, (local_id) program_arg.child_proc_number);
    client_third_phase(&client);
    AllHistory *all = &(client.all_history);
    print_history(all);
    client_destroy(&client);
    int status;
    for (size_t i = 0; i < program_arg.child_proc_number; ++i) {
        wait(&status);
    }
    logger_destroy();
    return 0;
}
