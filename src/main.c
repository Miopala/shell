#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "common.h"
#include "parser.h"
#include "builtin.h"
#include "executor.h"

void run_shell(FILE *input_stream, bool interactive)
{
  size_t size = 0;
  char *buffer = NULL;

  while (true)
  {
    if (interactive)
      write(STDOUT_FILENO, "wish> ", 6);
    ssize_t characters_read = getline(&buffer, &size, input_stream);

    if (characters_read == -1)
    {
      free(buffer);
      exit(0);
    }
    if (characters_read > 0 && buffer[characters_read - 1] == '\n')
      buffer[characters_read - 1] = '\0';

    char *expanded_buffer = expand_buffer(buffer, characters_read);
    if (expanded_buffer == NULL)
    {
      print_error();
      continue;
    }

    char *copy_buffer = expanded_buffer;
    while (copy_buffer != NULL)
    {
      char *cur_process_command = strsep(&copy_buffer, "&");
      if (strlen(cur_process_command) == 0)
      {
        continue;
      }

      Command cmd;
      if (!parse_segment(cur_process_command, &cmd))
      {
        print_error();
        continue;
      }

      if (handle_builtin(&cmd, buffer))
        continue;

      execute_command(&cmd);
    }
    free(expanded_buffer);
    while (wait(NULL) > 0)
      ;
  }
}

int main(int argc, char *argv[])
{
  shell_paths[0] = strdup("/bin");
  shell_paths[1] = NULL;
  path_count = 1;

  if (argc == 1)
  {
    run_shell(stdin, true);
  }
  else if (argc == 2)
  {
    FILE *batch_file = fopen(argv[1], "r");
    if (batch_file == NULL)
    {
      print_error();
      exit(1);
    }
    run_shell(batch_file, false);
    fclose(batch_file);
  }
  else
  {
    print_error();
    exit(1);
  }
  return 0;
}