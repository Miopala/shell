#include "builtin.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool handle_builtin(Pipeline *pipeline, char *buffer) {
    if (pipeline->cmd_count == 0)
        return false;

    Command *cmd = &pipeline->cmds[0];
    if (cmd->arg_count == 0 || cmd->args[0] == NULL)
        return false;

    if (strcmp(cmd->args[0], "cd") == 0 || strcmp(cmd->args[0], "exit") == 0 ||
        strcmp(cmd->args[0], "path") == 0) {
        if (pipeline->cmd_count > 1)
            return false;

        if (pipeline->input_file != NULL || pipeline->output_file != NULL) {
            print_error();
            return true;
        }

        if (strcmp(cmd->args[0], "exit") == 0) {
            if (cmd->arg_count != 1) {
                print_error();
                return true;
            }
            free(buffer);
            exit(0);
        }

        if (strcmp(cmd->args[0], "cd") == 0) {
            if (cmd->arg_count != 2) {
                print_error();
                return true;
            }
            chdir(cmd->args[1]);
            return true;
        }

        for (int i = 0; i < path_count; i++) {
            free(shell_paths[i]);
            shell_paths[i] = NULL;
        }
        path_count = 0;

        for (int i = 1; i < cmd->arg_count; i++) {
            if (path_count < MAX_PATHS - 1) {
                shell_paths[path_count++] = strdup(cmd->args[i]);
            }
        }
        shell_paths[path_count] = NULL;
        return true;
    }

    return false;
}