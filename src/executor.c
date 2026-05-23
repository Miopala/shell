#include "executor.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void execute_pipeline(Pipeline *pipeline) {
    if (pipeline->cmd_count == 0)
        return;

    char full_paths[MAX_COMMANDS][512];

    for (int i = 0; i < pipeline->cmd_count; i++) {
        Command *cur_cmd = &pipeline->cmds[i];
        if (cur_cmd->arg_count == 0 || cur_cmd->args[0] == NULL)
            return;

        bool found = false;
        for (int j = 0; j < path_count; j++) {
            snprintf(full_paths[i], sizeof(full_paths[i]), "%s/%s", shell_paths[j],
                     cur_cmd->args[0]);
            if (access(full_paths[i], X_OK) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            print_error();
            return;
        }
    }

    int pipefds[MAX_COMMANDS - 1][2];
    for (int i = 0; i < pipeline->cmd_count; i++) {
        if (i < pipeline->cmd_count - 1) {
            if (pipe(pipefds[i]) < 0) {
                print_error();
                if (i > 0) {
                    close(pipefds[i - 1][0]);
                }
                while (wait(NULL) > 0)
                    ;
                return;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            print_error();
            if (i > 0) {
                close(pipefds[i - 1][0]);
            }
            if (i < pipeline->cmd_count - 1) {
                close(pipefds[i][0]);
                close(pipefds[i][1]);
            }
            while (wait(NULL) > 0)
                ;
            return;
        } else if (pid == 0) {
            if (i == 0) {
                if (pipeline->input_file != NULL) {
                    int infd = open(pipeline->input_file, O_RDONLY);
                    if (infd < 0) {
                        print_error();
                        exit(1);
                    }
                    dup2(infd, STDIN_FILENO);
                    close(infd);
                }
            } else {
                dup2(pipefds[i - 1][0], STDIN_FILENO);
                close(pipefds[i - 1][0]);
            }

            if (i == pipeline->cmd_count - 1) {
                if (pipeline->output_file != NULL) {
                    int outfd = open(pipeline->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (outfd < 0) {
                        print_error();
                        exit(1);
                    }
                    dup2(outfd, STDOUT_FILENO);
                    close(outfd);
                }
            } else {
                close(pipefds[i][0]);
                dup2(pipefds[i][1], STDOUT_FILENO);
                close(pipefds[i][1]);
            }

            execv(full_paths[i], pipeline->cmds[i].args);
            print_error();
            exit(1);
        }

        if (i > 0) {
            close(pipefds[i - 1][0]);
        }
        if (i < pipeline->cmd_count - 1) {
            close(pipefds[i][1]);
        }
    }

    while (wait(NULL) > 0)
        ;
}