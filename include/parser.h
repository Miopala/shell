#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include <sys/types.h>

typedef struct
{
  char *args[MAX_ARGS];
  int arg_count;
  int position_of_redirection;
  int position_of_pipe;
  char *to_redirect;
} Command;

char *expand_buffer(const char *buffer, ssize_t characters_read);
bool parse_segment(char *cur_process_command, Command *cmd);

#endif