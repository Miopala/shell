#include "parser.h"
#include <stdlib.h>
#include <string.h>

char *expand_buffer(const char *buffer, ssize_t characters_read) {
    char *expanded_buffer = (char *)malloc(characters_read * 3 + 1);
    if (expanded_buffer == NULL)
        return NULL;

    ssize_t new_characters_read = 0;
    for (int i = 0; i < characters_read; i++) {
        if (buffer[i] == '>' || buffer[i] == '|' || buffer[i] == '<') {
            expanded_buffer[new_characters_read++] = ' ';
            expanded_buffer[new_characters_read++] = buffer[i];
            expanded_buffer[new_characters_read++] = ' ';
        } else {
            expanded_buffer[new_characters_read++] = buffer[i];
        }
    }
    expanded_buffer[new_characters_read] = '\0';
    return expanded_buffer;
}

static bool extract_redirection(Command *cmd, const char *operator, char **target_file) {
    int position = -1;
    int count = 0;

    for (int i = 0; i < cmd->arg_count; i++) {
        if (strcmp(cmd->args[i], operator) == 0) {
            count++;
            position = i;
        }
    }

    if (count > 1)
        return false;

    if (position != -1) {
        if (position == cmd->arg_count - 1)
            return false;

        *target_file = cmd->args[position + 1];
        if (strcmp(*target_file, "<") == 0 || strcmp(*target_file, ">") == 0 ||
            strcmp(*target_file, "|") == 0)
            return false;

        for (int i = position; i < cmd->arg_count - 2; i++) {
            cmd->args[i] = cmd->args[i + 2];
        }
        cmd->arg_count -= 2;
        cmd->args[cmd->arg_count] = NULL;
    }

    return true;
}

bool parse_pipeline(char *segment, Pipeline *pipeline) {
    pipeline->cmd_count = 0;
    pipeline->input_file = NULL;
    pipeline->output_file = NULL;
    char *copy_buffer = segment;

    while (copy_buffer != NULL) {
        char *cur_command = strsep(&copy_buffer, "|");
        if (pipeline->cmd_count >= MAX_COMMANDS)
            return false;

        Command *cur_cmd = &pipeline->cmds[pipeline->cmd_count];
        cur_cmd->arg_count = 0;
        char *copy_buffer2 = cur_command;
        while (copy_buffer2 != NULL) {
            char *cur_arg = strsep(&copy_buffer2, " \t");
            if (strlen(cur_arg) > 0) {
                if (cur_cmd->arg_count >= MAX_ARGS - 1)
                    return false;
                cur_cmd->args[cur_cmd->arg_count++] = cur_arg;
            }
        }
        if (cur_cmd->arg_count == 0) {
            return false;
        }
        pipeline->cmd_count++;
        cur_cmd->args[cur_cmd->arg_count] = NULL;
    }

    if (pipeline->cmd_count == 0)
        return false;

    for (int i = 0; i < pipeline->cmd_count; i++) {
        Command *cur_cmd = &pipeline->cmds[i];
        for (int j = 0; j < cur_cmd->arg_count; j++) {
            if (strcmp(cur_cmd->args[j], "<") == 0 && i != 0)
                return false;
            if (strcmp(cur_cmd->args[j], ">") == 0 && i != pipeline->cmd_count - 1)
                return false;
        }
    }

    if (!extract_redirection(&pipeline->cmds[0], "<", &pipeline->input_file))
        return false;

    if (!extract_redirection(&pipeline->cmds[pipeline->cmd_count - 1], ">", &pipeline->output_file))
        return false;

    if (pipeline->cmds[0].arg_count == 0 || pipeline->cmds[pipeline->cmd_count - 1].arg_count == 0)
        return false;

    return true;
}