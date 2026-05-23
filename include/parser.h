#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include <sys/types.h>

#define MAX_COMMANDS 20

typedef struct {
    char *args[MAX_ARGS];
    int arg_count;
} Command;

typedef struct {
    Command cmds[MAX_COMMANDS];
    int cmd_count;
    char *input_file;
    char *output_file;
} Pipeline;

char *expand_buffer(const char *buffer, ssize_t characters_read);
bool parse_pipeline(char *segment, Pipeline *pipeline);
#endif