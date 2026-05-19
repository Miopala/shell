#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cassert>
void print_error()
{
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

const int MAX_ARGS = 100;
char *args[MAX_ARGS];

const int MAX_PATHS = 100;

char *shell_paths[MAX_PATHS] = {strdup("/bin"), NULL};
int path_count = 1;

void run_shell(FILE *input_stream, bool interactive)
{
  size_t size = 0;
  char *buffer = NULL;

  while (true)
  {
    if (interactive)
      write(STDOUT_FILENO, "wish> ", strlen("wish> "));
    ssize_t characters_read = getline(&buffer, &size, input_stream);

    if (characters_read == -1)
    {
      free(buffer);
      exit(0);
    }
    if (characters_read > 0 && buffer[characters_read - 1] == '\n')
      buffer[characters_read - 1] = '\0';

    ssize_t new_characters_read = 0;

    char *expanded_buffer = (char *)malloc(characters_read * 3 + 1);
    if (expanded_buffer == NULL)
    {
      print_error();
      continue;
    }
    for (int i = 0; i < characters_read; i++)
    {
      if (buffer[i] == '>' || buffer[i] == '|')
      {
        expanded_buffer[new_characters_read++] = ' ';
        expanded_buffer[new_characters_read++] = buffer[i];
        expanded_buffer[new_characters_read++] = ' ';
      }
      else
        expanded_buffer[new_characters_read++] = buffer[i];
    }
    expanded_buffer[new_characters_read] = '\0';

    int arg_count = 0;
    char *copy_buffer = expanded_buffer;
    int position_of_redirection = -1;
    int position_of_pipe = -1;
    while (copy_buffer != NULL)
    {
      arg_count = 0;
      position_of_redirection = -1;
      position_of_pipe = -1;
      char *cur_process_command = strsep(&copy_buffer, "&");
      if (strlen(cur_process_command) == 0)
      {
        continue;
      }

      char *copy_buffer2 = cur_process_command;
      int redirections = 0, pipes = 0;
      while (copy_buffer2 != NULL)
      {
        char *cur_arg = strsep(&copy_buffer2, " \t");
        if (strlen(cur_arg) > 0)
        {
          if (arg_count >= MAX_ARGS - 1)
            break;
          args[arg_count++] = cur_arg;

          if (strcmp(cur_arg, ">") == 0)
          {
            redirections++;
            position_of_redirection = arg_count - 1;
          }
          else if (strcmp(cur_arg, "|") == 0)
          {
            pipes++;
            position_of_pipe = arg_count - 1;
          }
        }
      }

      if (redirections > 1 || pipes > 1)
      {
        print_error();
        continue;
      }

      char *to_redirect = NULL;

      if (position_of_redirection == 0 || (position_of_redirection != -1 && position_of_redirection != arg_count - 2))
      {
        print_error();
        continue;
      }

      if (position_of_pipe == 0 || position_of_pipe == arg_count - 1)
      {
        print_error();
        continue;
      }

      if (position_of_redirection < position_of_pipe && position_of_redirection != -1)
      {
        print_error();
        continue;
      }

      if (position_of_pipe != -1 && position_of_redirection != -1 && position_of_pipe + 1 == position_of_redirection)
      {
        print_error();
        continue;
      }

      if (position_of_redirection != -1)
      {
        to_redirect = args[position_of_redirection + 1];
        args[position_of_redirection] = NULL;
      }

      args[arg_count] = NULL;
      if (arg_count == 0)
        continue;

      char **argsA = NULL;
      char **argsB = NULL;

      if (position_of_pipe != -1)
      {
        argsA = &args[0];
        argsB = &args[position_of_pipe + 1];
        args[position_of_pipe] = NULL;

        if (strcmp(argsA[0], "cd") == 0 || strcmp(argsA[0], "exit") == 0 || strcmp(argsA[0], "path") == 0 ||
            strcmp(argsB[0], "cd") == 0 || strcmp(argsB[0], "exit") == 0 || strcmp(argsB[0], "path") == 0)
        {
          print_error();
          continue;
        }
      }

      if (position_of_pipe == -1)
      {

        if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "exit") == 0 || strcmp(args[0], "path") == 0)
        {
          if (to_redirect != NULL)
          {
            print_error();
            continue;
          }

          if (strcmp(args[0], "exit") == 0)
          {
            if (arg_count != 1)
            {
              print_error();
              continue;
            }
            free(buffer);
            exit(0);
          }

          if (strcmp(args[0], "cd") == 0)
          {
            if (arg_count != 2)
            {
              print_error();
              continue;
            }
            chdir(args[1]);
            continue;
          }

          // args here must be path
          for (int i = 0; i < path_count; i++)
          {
            free(shell_paths[i]);
            shell_paths[i] = NULL;
          }
          path_count = 0;

          for (int i = 1; i < arg_count; i++)
          {
            if (path_count < MAX_PATHS - 1)
            {
              shell_paths[path_count++] = strdup(args[i]);
            }
          }
          shell_paths[path_count] = NULL;
          continue;
        }
      }

      if (position_of_pipe != -1)
      {
        char full_pathA[512], full_pathB[512];
        bool foundA = false, foundB = false;
        for (int i = 0; i < path_count; i++)
        {
          snprintf(full_pathA, sizeof(full_pathA), "%s/%s", shell_paths[i], argsA[0]);
          if (access(full_pathA, X_OK) == 0)
          {
            foundA = true;
            break;
          }
        }

        for (int i = 0; i < path_count; i++)
        {
          snprintf(full_pathB, sizeof(full_pathB), "%s/%s", shell_paths[i], argsB[0]);
          if (access(full_pathB, X_OK) == 0)
          {
            foundB = true;
            break;
          }
        }

        if (!foundA || !foundB)
        {
          print_error();
          continue;
        }

        int pipefd[2];
        int pipe_fd = pipe(pipefd);

        if (pipe_fd < 0)
        {
          print_error();
          continue;
        }

        pid_t pidA = fork();

        if (pidA < 0)
        {
          print_error();
          close(pipefd[0]);
          close(pipefd[1]);
          continue;
        }
        else if (pidA == 0)
        {
          if (to_redirect != NULL)
          {
            // int redirectd = open(to_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            // if (redirectd < 0)
            // {
            //   print_error();
            //   exit(1);
            // }
            // dup2(redirectd, STDOUT_FILENO);
            // close(redirectd);

            assert(position_of_redirection > position_of_pipe);
          }
          close(pipefd[0]);
          // 0 read 1 write
          dup2(pipefd[1], STDOUT_FILENO);
          execv(full_pathA, argsA);
          print_error();
          exit(1);
        }

        pid_t pidB = fork();

        if (pidB < 0)
        {
          print_error();
          close(pipefd[0]);
          close(pipefd[1]);
          continue;
        }
        else if (pidB == 0)
        {
          if (to_redirect != NULL)
          {
            int redirectd = open(to_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (redirectd < 0)
            {
              print_error();
              exit(1);
            }
            dup2(redirectd, STDOUT_FILENO);
            close(redirectd);

            assert(position_of_redirection > position_of_pipe);
          }
          close(pipefd[1]);
          // 0 read 1 write
          dup2(pipefd[0], STDIN_FILENO);
          execv(full_pathB, argsB);
          print_error();
          exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);
        continue;
      }

      char full_path[512];
      bool found = false;
      for (int i = 0; i < path_count; i++)
      {
        snprintf(full_path, sizeof(full_path), "%s/%s", shell_paths[i], args[0]);
        if (access(full_path, X_OK) == 0)
        {
          found = true;
          break;
        }
      }

      if (!found)
      {
        print_error();
        continue;
      }

      pid_t pid = fork();

      if (pid < 0)
      {
        print_error();
      }
      else if (pid == 0)
      {
        if (to_redirect != NULL)
        {
          int redirectd = open(to_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (redirectd < 0)
          {
            print_error();
            exit(1);
          }
          dup2(redirectd, STDOUT_FILENO);
          close(redirectd);
        }
        execv(full_path, args);
        print_error();
        exit(1);
      }
    }
    free(expanded_buffer);
    while (wait(NULL) > 0)
      ;
    // for (int i = 0; i < arg_count; i++)
    // {
    //   write(STDOUT_FILENO, args[i], strlen(args[i]));
    //   write(STDOUT_FILENO, "\n", 1);
    // }
  }
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    // fprintf(stderr, "Interactive Mode Selected\n");
    run_shell(stdin, true);
  }
  else if (argc == 2)
  {
    // fprintf(stderr, "Batch Mode Selected\n");
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
