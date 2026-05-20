#include "parser.h"
#include <stdlib.h>
#include <string.h>

char *expand_buffer(const char *buffer, ssize_t characters_read)
{
  char *expanded_buffer = (char *)malloc(characters_read * 3 + 1);
  if (expanded_buffer == NULL)
    return NULL;

  ssize_t new_characters_read = 0;
  for (int i = 0; i < characters_read; i++)
  {
    if (buffer[i] == '>' || buffer[i] == '|')
    {
      expanded_buffer[new_characters_read++] = ' ';
      expanded_buffer[new_characters_read++] = buffer[i];
      expanded_buffer[new_characters_read++] = ' ';
    }
    else
    {
      expanded_buffer[new_characters_read++] = buffer[i];
    }
  }
  expanded_buffer[new_characters_read] = '\0';
  return expanded_buffer;
}

bool parse_segment(char *cur_process_command, Command *cmd)
{
  cmd->arg_count = 0;
  cmd->position_of_redirection = -1;
  cmd->position_of_pipe = -1;
  cmd->to_redirect = NULL;

  int redirections = 0, pipes = 0;
  char *copy_buffer2 = cur_process_command;

  while (copy_buffer2 != NULL)
  {
    char *cur_arg = strsep(&copy_buffer2, " \t");
    if (strlen(cur_arg) > 0)
    {
      if (cmd->arg_count >= MAX_ARGS - 1)
        break;
      cmd->args[cmd->arg_count++] = cur_arg;

      if (strcmp(cur_arg, ">") == 0)
      {
        redirections++;
        cmd->position_of_redirection = cmd->arg_count - 1;
      }
      else if (strcmp(cur_arg, "|") == 0)
      {
        pipes++;
        cmd->position_of_pipe = cmd->arg_count - 1;
      }
    }
  }

  if (redirections > 1 || pipes > 1)
    return false;

  if (cmd->position_of_redirection == 0 || (cmd->position_of_redirection != -1 && cmd->position_of_redirection != cmd->arg_count - 2))
    return false;

  if (cmd->position_of_pipe == 0 || cmd->position_of_pipe == cmd->arg_count - 1)
    return false;

  if (cmd->position_of_redirection < cmd->position_of_pipe && cmd->position_of_redirection != -1)
    return false;

  if (cmd->position_of_pipe != -1 && cmd->position_of_redirection != -1 && cmd->position_of_pipe + 1 == cmd->position_of_redirection)
    return false;

  if (cmd->position_of_redirection != -1)
  {
    cmd->to_redirect = cmd->args[cmd->position_of_redirection + 1];
    cmd->args[cmd->position_of_redirection] = NULL;
  }

  cmd->args[cmd->arg_count] = NULL;
  return true;
}