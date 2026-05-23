#include "common.h"
#include <string.h>
#include <unistd.h>

char *shell_paths[MAX_PATHS];
int path_count = 0;

void print_error(void) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}