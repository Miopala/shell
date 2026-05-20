#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define MAX_ARGS 100
#define MAX_PATHS 100

extern char *shell_paths[MAX_PATHS];
extern int path_count;

void print_error(void);

#endif