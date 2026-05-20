#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

void execute_command(Command *cmd)
{
  if (cmd->arg_count == 0 || cmd->args[0] == NULL)
    return;

  char **argsA = NULL;
  char **argsB = NULL;

  if (cmd->position_of_pipe != -1)
  {
    argsA = &cmd->args[0];
    argsB = &cmd->args[cmd->position_of_pipe + 1];
    cmd->args[cmd->position_of_pipe] = NULL;

    if (strcmp(argsA[0], "cd") == 0 || strcmp(argsA[0], "exit") == 0 || strcmp(argsA[0], "path") == 0 ||
        strcmp(argsB[0], "cd") == 0 || strcmp(argsB[0], "exit") == 0 || strcmp(argsB[0], "path") == 0)
    {
      print_error();
      return;
    }

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
      return;
    }

    int pipefd[2];
    int pipe_fd = pipe(pipefd);
    if (pipe_fd < 0)
    {
      print_error();
      return;
    }

    pid_t pidA = fork();
    if (pidA < 0)
    {
      print_error();
      close(pipefd[0]);
      close(pipefd[1]);
      return;
    }
    else if (pidA == 0)
    {
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[1]);
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
      return;
    }
    else if (pidB == 0)
    {
      if (cmd->to_redirect != NULL)
      {
        int redirectd = open(cmd->to_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (redirectd < 0)
        {
          print_error();
          exit(1);
        }
        dup2(redirectd, STDOUT_FILENO);
        close(redirectd);
      }
      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
      execv(full_pathB, argsB);
      print_error();
      exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    return;
  }

  char full_path[512];
  bool found = false;
  for (int i = 0; i < path_count; i++)
  {
    snprintf(full_path, sizeof(full_path), "%s/%s", shell_paths[i], cmd->args[0]);
    if (access(full_path, X_OK) == 0)
    {
      found = true;
      break;
    }
  }

  if (!found)
  {
    print_error();
    return;
  }

  pid_t pid = fork();
  if (pid < 0)
  {
    print_error();
  }
  else if (pid == 0)
  {
    if (cmd->to_redirect != NULL)
    {
      int redirectd = open(cmd->to_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (redirectd < 0)
      {
        print_error();
        exit(1);
      }
      dup2(redirectd, STDOUT_FILENO);
      close(redirectd);
    }
    execv(full_path, cmd->args);
    print_error();
    exit(1);
  }
}